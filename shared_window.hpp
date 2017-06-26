#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++17 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_SHARED_WINDOW $0x.cpp -o $0x.x && time mpirun -np 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHARED_WINDOW_HPP
#define BOOST_MPI3_SHARED_WINDOW_HPP

#include "../mpi3/window.hpp"
#include "../mpi3/communicator.hpp"
#include<memory>

namespace boost{
namespace mpi3{

struct shared_window : window{
	MPI_Aint size(int rank) const{
		MPI_Aint size;
		int disp_unit;
		void* baseptr;
		MPI_Win_shared_query(impl_, rank, &size, &disp_unit, &baseptr);
		return size;
	}
	MPI_Aint disp_unit(int rank) const{
		MPI_Aint size;
		int disp_unit;
		void* baseptr;
		MPI_Win_shared_query(impl_, rank, &size, &disp_unit, &baseptr);
		return size;
	}
	void* base(int rank) const{
		MPI_Aint size;
		int disp_unit;
		void* baseptr;
		MPI_Win_shared_query(impl_, rank, &size, &disp_unit, &baseptr);
		return baseptr;
	}
};

template<class T>
shared_window communicator::allocate_shared(MPI_Aint size, int disp_unit){
	void* base_ptr;
	shared_window ret;
	MPI_Win_allocate_shared(size, disp_unit, MPI_INFO_NULL, impl_, &base_ptr, &ret.impl_);
	return ret;
}

namespace intranode{

template<class T> struct array_ptr;

template<>
struct array_ptr<const void>{
	using T = const void;
	std::shared_ptr<shared_window> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
};

template<>
struct array_ptr<void>{
	using T = void;
	std::shared_ptr<shared_window> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
};

template<class T>
struct array_ptr{
	std::shared_ptr<shared_window> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr) : offset(0){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
	T& operator*() const{return *((T*)(wSP_->base(0)) + offset);}
	T& operator[](int idx) const{return ((T*)(wSP_->base(0)) + offset)[idx];}
	T* operator->() const{return (T*)(wSP_->base(0)) + offset;}
	operator array_ptr<void const>() const{
		array_ptr<void const> ret;
		ret.wSP_ = wSP_;
		return ret;
	}
	array_ptr operator+(std::ptrdiff_t d) const{
		array_ptr ret(*this);
		ret += d;
		return ret;
	}
	std::ptrdiff_t operator-(array_ptr other) const{
		return offset - other.offset;
	}
	array_ptr& operator--(){--offset; return *this;}
	array_ptr& operator++(){++offset; return *this;}
	array_ptr& operator-=(std::ptrdiff_t d){offset -= d; return *this;}
	array_ptr& operator+=(std::ptrdiff_t d){offset += d; return *this;}
	bool operator==(array_ptr<T> const& other) const{
		return wSP_->base(0) == other.wSP_->base(0) and offset == other.offset;
	}
	bool operator!=(array_ptr<T> const& other) const{return not((*this)==other);}
};

template<class T> struct allocator{
	mpi3::communicator& comm_;
	allocator(mpi3::communicator& comm) : comm_(comm){}

	array_ptr<T> allocate(std::size_t n, const void* hint = 0){
		array_ptr<T> ret;
		ret.wSP_ = std::make_shared<shared_window>(comm_.allocate_shared(comm_.rank()==0?n*sizeof(T):1));
		return ret;
	}
	void deallocate(array_ptr<T>& ptr, std::size_t){ptr.wSP_.reset();}

	using value_type = T;
	using pointer = array_ptr<T>;
	using reference = T&;
};

}


}}

#ifdef _TEST_BOOST_MPI3_SHARED_WINDOW
#include<iostream>
#include<algorithm> // generate
#include<random>
#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/mutex.hpp"
#include <mutex>
#include<thread> 
#include<chrono>
#include "../mpi3/mutex.hpp"
#include "../mpi3/shm/vector.hpp"

int rand(int lower, int upper){
	static std::random_device rd;
	static std::mt19937 rng(rd());
	static std::uniform_int_distribution<int> uni(lower, upper); 
	return uni(rng);
}
int rand(int upper = RAND_MAX){return rand(0, upper);}

namespace mpi3 = boost::mpi3;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::shm::vector<double> v(10, world);
	if(world.rank() == 0){
		for(int i = 0; i != 10; ++i){
			std::this_thread::sleep_for(std::chrono::milliseconds(rand(100)));
			v[i] = (i+1)*10;
		}
	}

	mpi3::mutex m(world);
	m.lock();
	if(world.rank() == 1){
		for(int i = 0; i != 10; ++i){
			v[i] = -(i+1)*10;
			std::this_thread::sleep_for(std::chrono::milliseconds(rand(1000)));
		}
	}
	m.unlock();

	world.barrier();
	if(world.rank() == 2){
		for(int i = 0; i != 10; ++i){
			std::cout << v[i] << " " << std::flush;
		}
	}

}

#endif
#endif

