#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -n 4 $0x.x $@ && rm -f $0x.x; exit
#endif
//  (C) Copyright Alfredo A. Correa 2018.

#include "../../mpi3/environment.hpp"
#include "../../mpi3/group.hpp"
#include "../../mpi3/communicator.hpp"

#include "../../mpi3/main.hpp"

#include<iostream>

using std::cout;
namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], mpi3::communicator world){
	
	mpi3::group wg{world};
	mpi3::communicator w2 = wg;
	assert(w2.rank() == world.rank());

	mpi3::communicator half = world/2;
	mpi3::group hg{half};

	mpi3::communicator h2 = hg;
	assert(half.rank() == h2.rank());
	return 0;
}

