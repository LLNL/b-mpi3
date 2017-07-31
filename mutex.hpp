#if COMPILATION_INSTRUCTIONS
(echo "#include \""$0"\"" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_MUTEX $0x.cpp -o $0x.x && time mpirun -np 8 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_MUTEX_HPP
#define BOOST_MPI3_MUTEX_HPP

#include<mpi.h>

#include "../mpi3/window.hpp"
#include "../mpi3/detail/basic_mutex.hpp"

namespace boost{
namespace mpi3{

using mutex = detail::basic_mutex<mpi3::window>;

template<class T>
struct atomic{
	int rank_;
	T* addr_;
	communicator& comm_;
//	T counter = -999;
	window win_;

	atomic(T const& value, communicator& comm, int rank = 0) : 
		comm_(comm),
		rank_(rank),
		addr_(static_cast<T*>(comm.rank()==rank?mpi3::malloc(sizeof(T)):nullptr)),
		win_(addr_, addr_?sizeof(T):0, comm_)
	{
		if(addr_) new (addr_) T(value);
//		if(addr_) *addr_ = value;
	}
	atomic& operator+=(T const& t){
		win_.lock_exclusive(rank_);
		win_.fetch_sum_value(t, *addr_, rank_);
	//	win_.fetch_sum_value(t, counter, rank_);
		win_.unlock(rank_);
		return *this;
	}
	atomic& operator-=(T const& t){return operator+=(-t);}
	atomic& operator*=(T const& t){
		win_.lock_exclusive(rank_);
		win_.fetch_prod_value(t, *addr_, rank_);
	//	win_.fetch_prod_value(t, counter, rank_);
		win_.unlock(rank_);
		return *this;
	}
	atomic& operator/=(T const& t);
	atomic& operator=(T const& t){
		win_.lock_exclusive(rank_);
		win_.fetch_replace_value(t, *addr_, rank_);
	//	win_.fetch_replace_value(t, counter, rank_);
		win_.unlock(rank_);
		return *this;
	}
//	T const& load() const{return counter;}
	operator T(){
		T t;
		win_.lock_exclusive(0);
		win_.put_value(*addr_, 0, comm_.rank());
		win_.get_n(&t, 1, 0);
		win_.unlock(0);
		return t;
	}
	~atomic(){
		comm_.barrier();
		if(addr_) boost::mpi3::free(addr_);
	}
};

template<> atomic<double>& atomic<double>::operator/=(double const& d){return operator*=(1./d);}
template<> atomic<float >& atomic<float >::operator/=(float  const& d){return operator*=(1./d);}

}}

#ifdef _TEST_BOOST_MPI3_MUTEX

#include "alf/boost/mpi3/main.hpp"

#include<thread>
#include<random>
#include<chrono>

#include<iostream>
#include <mutex>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	{
		mpi3::atomic<int> counter(0, world);
		counter += 1;
		if(counter + 1 == world.size()) cout << "Process #" << world.rank() << " did the last updated" << std::endl;
	}
	{
		mpi3::atomic<int> counter(0, world);
		counter += 1;
		world.barrier();
		{
			mpi3::mutex m(world);
			std::lock_guard<mpi3::mutex> lock(m);
			cout << "on process " << world.rank() << " counter = " << (int)counter << std::endl;
		}
	}
	{
		mpi3::mutex m(world);
		std::lock_guard<mpi3::mutex> lock(m);

		cout << "locked from " << world.rank() << '\n';
		cout << "never interleaved " << world.rank() << '\n';
		cout << "forever blocked " << world.rank() << '\n';
		cout << std::endl;
	}
}

#endif
#endif

