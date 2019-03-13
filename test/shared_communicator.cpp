#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wall -Wextra -Wfatal-errors $0 -o $0x.x && time mpirun -n 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/shared_communicator.hpp"
#include "../../mpi3/ostream.hpp"

#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;

void check_isa_communicator(mpi3::communicator const&){}
void check_isa_shared_comm (mpi3::shared_communicator const&){}

int mpi3::main(int, char*[], mpi3::communicator world){

	mpi3::ostream wout(world);

	mpi3::shared_communicator node = world.split_shared();
	mpi3::shared_communicator node2 = node.split(node.rank() % 2, node.rank());

	wout << "I am rank " << node.rank() << " in " << node.name() << " and rank " << node2.rank() << " in " << node2.name() << std::endl;

	check_isa_shared_comm(node);
	check_isa_communicator(node);

	mpi3::communicator virtual_node{node};
	mpi3::communicator virtual_node2;
	virtual_node2 = node2;

	return 0;
}

