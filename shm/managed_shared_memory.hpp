#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_SHM_MANAGED_SHARED_MEMORY $0x.cpp -o $0x.x -lrt -lboost_system && time mpirun -np 10 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHM_MANAGED_SHARED_MEMORY_HPP
#define BOOST_MPI3_SHM_MANAGED_SHARED_MEMORY_HPP

#include <boost/pool/simple_segregated_storage.hpp>
#include "../../mpi3/shared_window.hpp"
#include<random>
#include<thread>

int rand(int lower, int upper){
	static std::random_device rd;
	static std::mt19937 rng(rd());
	static std::uniform_int_distribution<int> uni(lower, upper); 
	return uni(rng);
}
int rand(int upper = RAND_MAX){return rand(0, upper);}

namespace boost{
namespace mpi3{
namespace shm{

struct managed_shared_memory{
	communicator& comm_;
	shared_window sw_;
	boost::simple_segregated_storage<std::size_t> storage_;
	managed_shared_memory(communicator& comm, mpi3::size_t n) : 
		comm_(comm), 
		sw_(comm.make_shared_window<char>(comm.rank()?0:n))
	{
		storage_.add_block(sw_.base(0), sw_.size(0), n);
	}
	template<class T>
	T* malloc(){
		return (T*)storage_.malloc_n(1, sizeof(T));
	}
	template<class T, class... Args>
	T* construct(Args&&... args){
		sw_.lock_all();//exclusive(0);
		sw_.sync();
		T* p = malloc<T>();
		if(comm_.rank() == 0) new(p) T(std::forward<Args>(args)...);
		std::this_thread::sleep_for(std::chrono::seconds(rand(10)));
	//	sw_.unlock(0);
		sw_.unlock_all();
		return p;
	}
	template<class T>
	void destroy(T* ptr){
		sw_.lock_exclusive(0);
		if(comm_.rank()==0) ptr->~T();
		storage_.free_n(ptr, 1, sizeof(T));
		sw_.unlock(0);
	}
};

}}}

#ifdef _TEST_BOOST_MPI3_SHM_MANAGED_SHARED_MEMORY

#include "alf/boost/mpi3/main.hpp"
#include<atomic>

namespace mpi3 = boost::mpi3;
using std::cout; 

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::shm::managed_shared_memory msm(world, 10000);

	std::atomic<int>& i = *msm.construct<std::atomic<int>>(0);
	world.barrier();

	++i; std::this_thread::sleep_for(std::chrono::seconds(rand(10)));

	if(world.rank() == 0){
		int snapshot = i;
		cout << "snapshot " << snapshot << " size " << world.size() << std::endl;
	}

	world.barrier();
	msm.destroy<std::atomic<int>>(&i);

	return 0;
}
#endif
#endif

