#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++17 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_SHM_VECTOR $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHM_VECTOR_HPP
#define BOOST_MPI3_SHM_VECTOR_HPP

#include "../../mpi3/shared_window.hpp"
#include<boost/container/vector.hpp>

namespace boost{
namespace mpi3{
namespace shm{

template<class T>
using allocator = boost::mpi3::intranode::allocator<T>;

template<class T>
using vector = boost::container::vector<T, boost::mpi3::shm::allocator<T>>;

}}}

#ifdef _TEST_BOOST_MPI3_SHM_VECTOR

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

int mpi3::main(int argc, char* argv[], boost::mpi3::communicator const& world){

	boost::mpi3::shm::vector<double> v(10, world);
	assert( std::equal(std::next(v.begin()), v.end(), v.begin()) );

	boost::mpi3::mutex m(world);
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
		for(int i = 0; i != 10; ++i){
			std::cout << v[i] << " ";
		}
		std::cout << std::endl;
	}
//	assert( std::all_of(v.begin(), v.end(), [&](auto&& e){return e == v[0];} );
	assert( std::equal(std::next(v.begin()), v.end(), v.begin()) );
}

#endif
#endif

