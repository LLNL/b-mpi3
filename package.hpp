#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++17 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_PACKAGE $0x.cpp -o $0x.x && time mpirun -np 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_PACKAGE_HPP
#define BOOST_MPI3_PACKAGE_HPP

#include "../mpi3/communicator.hpp"

namespace boost{
namespace mpi3{

struct package{
	package(package const&) = delete;
	package(package&&) = default;
	communicator& comm_;
	std::vector<char> buffer_;
	std::ptrdiff_t size() const{return buffer_.size();}
	int in_;
	int out_;
	package(communicator& comm, int n = 1024) : comm_(comm), buffer_(n), in_(0), out_(0){}
	void clear(){buffer_.clear(); in_ = 0; out_ = 0;}
	template<class T, class Size>
	void pack_n(T const* first, Size count){
		int size = comm_.pack_size<T>()*count;
		buffer_.resize(in_ + size);
		auto curr = std::addressof(buffer_[in_]);
		auto end = comm_.pack_n(first, count, curr);
		in_ += end - curr;
	}
	template<class T>
	package& operator<<(T const& t){
		pack_n(std::addressof(t), 1);
		return *this;
	}
	template<class T, class Size>
	void unpack_n(T* first, Size count){
		auto curr = std::addressof(buffer_[out_]);
		auto end = comm_.unpack_n(first, count, curr);
		out_ += end - curr;
	}
	template<class T>
	package& operator>>(T& t){
		unpack_n(std::addressof(t), 1);
		return *this;
	}
	package const& send(int dest, int tag = 0) const{
		comm_.send_packed_n(buffer_.data(), in_, dest, tag);
		return *this;
	}
	package& receive(int source = MPI_ANY_SOURCE, int tag = MPI_ANY_TAG){
		int n = comm_.probe(source, tag).count();
		buffer_.resize(n);
		comm_.receive_packed_n(buffer_.data(), n, source, tag);
		return *this;
	}
	package& broadcast(int root = 0){
		comm_.broadcast_n(&in_, 1, root);
		buffer_.resize(in_);
		comm_.broadcast_n(buffer_.data(), in_, root);
		return *this;
	}
	template<class T> int size(int n = 1) const{return comm_.pack_size<T>(n);}
};

package communicator::make_package(int n){return package(*this, n);}

}}

#ifdef _TEST_BOOST_MPI3_PACKAGE

#include "alf/boost/mpi3/main.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	if(world.rank() == 0){
		char buff[1000];
		int i = 12;
		int j = 13;
		auto 
		end = world.pack_n(&i, 1, buff);
		end = world.pack_n(&j, 1, end);
		world.send_packed(buff, end, 1); //world.send_packed_n(buff, end - buff, 1);
		world.send_packed_n(buff, 1000, 2);
	}else if(world.rank() == 1){
		std::vector<int> v(2);
		world.receive(v.begin(), v.end(), 0);
		assert(v[0] == 12);
		assert(v[1] == 13);
	}else if(world.rank() == 2){
		char buff[1000];
		world.receive_packed(buff, 0);
		int i = -1;
		int j = -1;
		auto 
		end = world.unpack_n(&i, 1, buff);
		end = world.unpack_n(&j, 1, end);
		assert(i == 12);
		assert(j == 13);
	}
	world.barrier();

	if(world.rank() == 0){
		auto p = world.make_package();
		int i = 12;
		int j = 13;
		(p << i << j).send(1).send(2);
	//	p.send(1);
	//	p.send(2);
	}else if(world.rank() == 1){
		std::vector<int> v(2, -1);
		world.receive(v.begin(), v.end(), 0);
		assert(v[0] = 12);
		assert(v[1] == 13);
	}else if(world.rank() == 2){
		auto p = world.make_package(); //	boost::mpi3::package p(world);
		int i = -1;
		int j = -1;
		p.receive(0) 
			>> i 
			>> j
		;
		assert(i == 12);
		assert(j == 13);
	}
}

#endif
#endif

