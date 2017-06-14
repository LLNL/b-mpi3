#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wall -Wfatal-errors $0 -o $0x.x && time mpirun -np 4s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	world.barrier();
	cout << "I am " << world.rank() << " of " << world.size() << '\n';
	return 0;
}

