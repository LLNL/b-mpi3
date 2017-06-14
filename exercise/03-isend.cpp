#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++17 -Wfatal-errors $0 -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/version.hpp"
#include "alf/boost/mpi3/processor_name.hpp"

#include<iostream>

using std::cout;
using std::endl;

int boost::mpi3::main(int argc, char* argv[], boost::mpi3::communicator const& world){
	if(world.size() % 2 != 0){
		if(world.master()) cout << "Quitting. Need an even number of tasks: numtasks = " << world.size() << "\n";
		return 1;
	}

	int message = -1;
	cout << "Hello from task " << world.rank() << " on host " << boost::mpi3::processor_name() << "\n";
	if(world.master()) cout << "MASTER: number of mpi tasks is " << world.size() << "\n";

	int partner = world.rank()<world.size()/2?world.rank() + world.size()/2:world.rank()-world.size()/2;

	boost::mpi3::wait(
		world.ireceive(message, partner, 1),
		world.isend(world.rank(), partner, 1)
	);

	cout << "Task " << world.rank() << " is partner with " << message << endl;

}

