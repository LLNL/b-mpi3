#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && mpicxx -O3 -fpermissive -std=c++14 -Wall -Wfatal-errors -D_TEST_BOOST_MPI3_SHM_VECTOR $0x.cpp -o $0x.x && time mpirun -np 10 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHM_VECTOR_HPP
#define BOOST_MPI3_SHM_VECTOR_HPP

//#include "../../mpi3/shm/memory.hpp"
//#include<boost/container/vector.hpp>
#include "../shared_window.hpp"

namespace boost{
namespace mpi3{
namespace shm{

//template<class T>
//using allocator = boost::mpi3::intranode::allocator<T>;

//template<class T>
//using vector = boost::container::vector<T, boost::mpi3::shm::allocator<T>>;

}}}

#ifdef _TEST_BOOST_MPI3_SHM_VECTOR
#include "../../mpi3/main.hpp"

namespace mpi3 = boost::mpi3; 
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	mpi3::shared_communicator node = world.split_shared();
	using Alloc = boost::mpi3::intranode::allocator<double>;

	std::vector<double, Alloc> v(100, node);
	node.barrier();
	v[node.rank()] = node.rank()*10.;
	node.barrier();
	for(int i = 0; i != node.size(); ++i){
		assert(v[i] == i*10.);
	}
	node.barrier();
	return 0;
}

#if 0
#include<iostream>
#include<algorithm> // generate
#include<random>
#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/mutex.hpp"
#include <mutex>
#include<thread> 
#include<chrono>
#include<cassert>

int rand(int lower, int upper){
	static std::random_device rd;
	static std::mt19937 rng(rd());
	static std::uniform_int_distribution<int> uni(lower, upper); 
	return uni(rng);
}
int rand(int upper = RAND_MAX){return rand(0, upper);}

namespace mpi3 = boost::mpi3;
using std::cout; 

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::shm::managed_shared_memory mpi3mshm(world);

	using elem = double;

	mpi3::shm::vector<elem> v(10, mpi3mshm.get_allocator<elem>());
	assert(not v.empty() and v.front() == 0 and std::equal(std::next(v.begin()), v.end(), v.begin()) );

	mpi3::mutex m(world);
	std::this_thread::sleep_for(std::chrono::milliseconds(rand(10)));
	{
		std::lock_guard<mpi3::mutex> lock(m); // m.lock();
		for(int i = 0; i != 10; ++i){
			v[i] = world.rank();
			std::this_thread::sleep_for(std::chrono::milliseconds(rand(10)));
		} //	m.unlock();
	}
	world.barrier();

	if(world.rank() == 0){
		for(int i = 0; i != 10; ++i)
			cout << v[i] << " ";
		cout << std::endl;
	}
	assert( std::equal(std::next(v.begin()), v.end(), v.begin()) );

	v.resize(15);

	return 0;
#if 0

	mpi3::mutex m(world);
	std::this_thread::sleep_for(std::chrono::milliseconds(rand(10)));
	{
		std::lock_guard<boost::mpi3::mutex> lock(m);
	//	m.lock();
		for(int i = 0; i != 10; ++i){
			v[i] = world.rank();
			std::this_thread::sleep_for(std::chrono::milliseconds(rand(10)));
		}
	//	m.unlock();
	}

	world.barrier();

	if(world.rank() == 0){
		for(int i = 0; i != 10; ++i)
			cout << v[i] << " ";
		cout << std::endl;
	}
	assert( std::equal(std::next(v.begin()), v.end(), v.begin()) );
//	v.resize(2);
#endif
}
#endif

#endif
#endif

