#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wall -Wfatal-errors $0 -o $0x.x -lboost_serialization && time mpirun -np 4 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"
#include "alf/boost/mpi3/process.hpp"
#include "alf/boost/mpi3/shm/vector.hpp"
#include "alf/boost/mpi3/mutex.hpp"
#include<random>
#include<thread> //sleep_for
#include<mutex> //lock_guard

int rand(int lower, int upper){
	static std::mt19937 rng(std::random_device{}());
	static std::uniform_int_distribution<int> uni(lower, upper); 
	return uni(rng);
}
int rand(int upper = RAND_MAX){return rand(0, upper);}

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

//	mpi3::shm::vector<double>::allocator_type alloc(world);
	mpi3::shm::vector<double> v(10, world);
	assert( std::equal(std::next(v.begin()), v.end(), v.begin()) );

	mpi3::mutex m(world);
	std::this_thread::sleep_for(std::chrono::milliseconds(rand(10)));
	{
		std::lock_guard<mpi3::mutex> lock(m);
	//	m.lock();
		for(int i = 0; i != 10; ++i){
			v[i] = world.rank();
			std::this_thread::sleep_for(std::chrono::milliseconds(rand(10))); // slow down the writing
		}
	//	m.unlock();
	}

	world.barrier();

	if(world.rank() == 0){
		for(int i = 0; i != 10; ++i)
			cout << v[i] << " ";
		cout << std::endl;
	}
	// check that only one process had exclusive access 
	assert( std::equal(std::next(v.begin()), v.end(), v.begin()) );

	return 0;
}

