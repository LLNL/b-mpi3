#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

#include <chrono> //literals
#include <thread> //sleep_for

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	assert( world.size() == 8 );
	
	mpi3::communicator third = world / 3;

	cout 
		<< "I am rank " << world.rank() << " in " << world.name() << ", "
		<< "I am also " << third.rank() << " in " << third.name() << '\n'
	;
	
	return 0;
}

