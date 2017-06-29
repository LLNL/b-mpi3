#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 5 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"
#include "alf/boost/mpi3/shared_window.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char *argv[], mpi3::communicator& world){

	int size = 100;
	mpi3::shared_window sw(world, world.rank()==0?100*sizeof(double):0);
	int* arr = (int*)sw.base(0);
//	int size = sw.size(0)/sizeof(double);

	cout << "In rank " << world.rank() << " baseptr = " << arr << std::endl;

	for(int i = world.rank(); i < size; i+=world.size())
		arr[i] = world.rank();

	world.barrier();

	if(world.rank() == 0){
		for(int i = 0; i < size; i++) cout << arr[i];
		cout << '\n';
	}

	return 0;
}

