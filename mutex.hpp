#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++17 -Wfatal-errors -D_TEST_BOOST_MPI3_MUTEX $0x.cpp -o $0x.x && time mpirun -np 8 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_MUTEX_HPP
#define BOOST_MPI3_MUTEX_HPP

#include<mpi.h>
#include "../mpi3/window.hpp"
#include "alf/boost/mpi3/allocator.hpp"

#include<cstring>

namespace boost{
namespace mpi3{

struct mutex{ //https://gist.github.com/aprell/1486197#file-mpi_mutex-c-L61
	static int tag_counter;
	using flag_t = unsigned char;

	communicator& comm_;
	int rank_; //home
	flag_t* addr_; //wait_list?
	window win_;
//	int tag_;

	mutex(mutex&&) = delete;
	mutex(mutex const&) = delete;
	mutex& operator=(mutex const&) = delete;
	mutex& operator=(mutex&&) = delete;

	mutex(communicator& comm, int rank = 0) : 
		comm_(comm),
		rank_(rank), 
		addr_((comm.rank() == rank)?(flag_t*)boost::mpi3::malloc(sizeof(flag_t)*comm.size()):nullptr),
		win_(addr_, addr_?1:0, comm_)
	{
		if(addr_) std::memset(addr_, 0, comm.size());
	//	tag_ = tag_counter;
		++tag_counter;
		comm.barrier();
	}

	void lock(){
		flag_t wait_list[comm_.size()];
		flag_t lock = 1;
		win_.lock_exclusive(rank_);
		win_.put_n(&lock, 1, rank_, comm_.rank());
		win_.get_n(wait_list, comm_.size(), rank_);
		win_.unlock(rank_);
		for(int i = 0; i != comm_.size(); ++i){
			if(wait_list[i] == 1 and i != comm_.rank()){
				comm_.receive_n(&lock, 0/*, MPI_ANY_SOURCE, tag_*/); //dummy receive
				break;
			}
		}
	}
	bool try_lock(){
		flag_t wait_list[comm_.size()];
		flag_t lock = 1;
		win_.lock_exclusive(rank_);
		win_.put_n(&lock, 1, rank_, comm_.rank());
		win_.get_n(wait_list, comm_.size(), rank_);
		win_.unlock(rank_);
		for(int i = 0; i != comm_.size(); ++i){
			if(wait_list[i] == 1 and i != comm_.rank()){
				return false;
			}
		}
		return true;
	}
	void unlock(){
		flag_t wait_list[comm_.size()];
		flag_t lock = 0;

		win_.lock_exclusive(rank_);
		win_.put_n(&lock, 1, rank_, comm_.rank());
		win_.get_n(wait_list, comm_.size(), rank_);
		win_.unlock(rank_);

		for(int i = 0; i != comm_.size(); ++i){
			int next = (comm_.rank() + i +1) % comm_.size();
			if(wait_list[next] == 1){
				comm_.send_n(&lock, 0, next/*, tag_*/);
				break;
			}
		}
	}
	~mutex(){
		comm_.barrier();
		if(addr_) boost::mpi3::free(addr_);
	}
};

int mutex::tag_counter = 11023;

template<class T>
struct atomic{
	int rank_;
	T* addr_;
	communicator& comm_;
	T counter = -999;
	window win_;

	atomic(T const& value, communicator& comm, int rank = 0) : 
		comm_(comm),
		rank_(rank),
		addr_((comm.rank() == rank)?(T*)boost::mpi3::malloc(sizeof(T)):nullptr),
		win_(addr_, addr_?1:0, comm_)
	{
		if(addr_) *addr_ = value;
	}
	atomic& operator+=(T const& t){
		win_.lock_exclusive(rank_);
		win_.fetch_sum_value(t, counter, rank_);
		win_.unlock(rank_);
		return *this;
	}
	atomic& operator-=(T const& t){return operator+=(-t);}
	atomic& operator*=(T const& t){
		win_.lock_exclusive(rank_);
		win_.fetch_prod_value(t, counter, rank_);
		win_.unlock(rank_);
		return *this;
	}
	atomic& operator/=(T const& t);
	atomic& operator=(T const& t){
		win_.lock_exclusive(rank_);
		win_.fetch_replace_value(t, counter, rank_);
		win_.unlock(rank_);
		return *this;
	}
	T const& load() const{return counter;}
	operator T const&() const{return counter;}
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
		boost::mpi3::atomic<int> counter(0, world);
		counter += 1;
		if(counter + 1 == world.size()) cout << "Process #" << world.rank() << " did the last updated" << std::endl;
	}
	{
		boost::mpi3::mutex m(world);
		std::lock_guard<boost::mpi3::mutex> lock(m);

		cout << "locked from " << world.rank() << '\n';
		cout << "never interleaved " << world.rank() << '\n';
		cout << "forever blocked " << world.rank() << '\n';
		cout << std::endl;
	}
}

#endif
#endif

