#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wall -Wextra `#-Wfatal-errors` -I$HOME/prj/alf $0 -o $0x.x && mpirun -n 4 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/shm/allocator.hpp"
#include "../../mpi3/main.hpp"
#include "../../mpi3/ostream.hpp"

#include "boost/multi/array.hpp"

namespace mpi3 = boost::mpi3;
namespace multi = boost::multi;

int mpi3::main(int, char*[], mpi3::communicator world){

	mpi3::shared_communicator node = world.split_shared();

	multi::array<double, 2, mpi3::shm::allocator<double>> A({16, 16}, node);	
	assert(A[2][2] == 0);
	node.barrier();
	if(node.root()) A[2][2] = 3.14;
	node.barrier();
	assert(A[2][2] == 3.14);

	multi::array<double, 2, mpi3::shm::allocator<double>> B = A;
	assert(A[2][2] == 3.14);
	assert(B[2][2] == 3.14);
	
	multi::array<double, 2, mpi3::shm::allocator<double>> C({1, 1}, node);
	C = A;
	assert(C[2][2] == 3.14);

	node.barrier();

	return 0;
}

