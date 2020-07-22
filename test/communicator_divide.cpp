#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -Wall -Wextra $0 -o $0x &&mpirun -n 3 valgrind --suppressions=communicator_main.cpp.openmpi.supp $0x&&rm $0x;exit
#endif
// © Alfredo Correa 2018-2020
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/main.hpp"

namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], mpi3::communicator world){
	assert( world.size() == 3 );
	return 0;
}

