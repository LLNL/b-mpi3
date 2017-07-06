#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wall `#-Wfatal-errors` $0 -o $0x.x -lboost_serialization && time mpirun -np 16 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/shm/vector.hpp"

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"
#include "alf/boost/mpi3/mutex.hpp"

#include<chrono>
#include<mutex> // lock_guard
#include<random>
#include<thread> // sleep_for

int rand(int lower, int upper){
	static std::random_device rd;
	static std::mt19937 rng(rd());
	static std::uniform_int_distribution<int> uni(lower, upper); 
	return uni(rng);
}
int rand(int upper = RAND_MAX){return rand(0, upper);}

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator& world){

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

	return 0;
}

