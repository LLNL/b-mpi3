#if COMPILATION_INSTRUCTIONS
 mpic++ -g -O3 -Wall -Wextra $0 -o $0x&&(mpirun -n 3 valgrind                                 --suppressions=$0.openmpi.supp $0x)&&rm $0x;exit
#mpic++ -g -O3 -Wall -Wextra $0 -o $0x&&(mpirun -n 3 valgrind --gen-suppressions=all $0x 2>&1|grep -v '==' > $0.openmpi.supp    )&&rm $0x;exit
#endif
// © Alfredo A. Correa 2018-2020

#include "../../mpi3/communicator.hpp"
#include "../../mpi3/main.hpp"

#include<complex>
#include<string>
#include<future>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){
	assert( world.size() == 3 );
	return 0;
}

