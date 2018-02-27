#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -I${HOME}/prj/ -Wfatal-errors $0 -o $0x.x && time mpirun -np 5 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/environment.hpp"
#include "alf/boost/mpi3/shared_window.hpp"

#include<iostream>

using std::cout;
using std::endl;

namespace mpi3 = boost::mpi3;

int main(int argc, char* argv[]){
	mpi3::environment env(argc, argv); // was   MPI_Init(&argc,&argv);
	auto world = env.world();          // was auto& world = boost::mpi3::world;
	mpi3::shared_communicator node = world.split_shared();
	cout<<" rank:  " <<world.rank() <<endl;
	mpi3::intranode::allocator<double> A(node);
	mpi3::intranode::allocator<double>::pointer data = A.allocate(8);
	if(node.root()) data[3] = 3.4;
	node.barrier();
	if(node.rank()) assert(data[3] == 3.4);
	node.barrier();
	A.deallocate(data, 8);
	return 0;                           // was MPI_Finalize();
}
