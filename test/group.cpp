#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wfatal-errors $0 -o $0x.x && time mpirun -n 8 $0x.x $@ && rm -f $0x.x; exit
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
	mpi3::communicator w1 = world;

	assert( w1.size() == world.size() );
	assert( w1.rank() == world.rank() );

	mpi3::group g1 = w1;

	assert( g1.rank() == w1.rank() );

	mpi3::communicator w2 = w1.create(g1);

	assert( w2.size() == w1.size() );
	assert( w2.rank() == w1.rank() );
	assert( w2.rank() == world.rank() );
	return 0;
}

