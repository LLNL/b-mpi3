#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wall -Wfatal-errors $0 -o $0x.x && time mpirun -np 2s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	std::vector<int> sizes = { 100, 64*1024, 128*1024 };
	int NUM_REPS = 5;

	std::vector<int> buf(sizes[sizes.size() - 1]);

	for(int n=0; n != (int)sizes.size(); n++){
		if (world.rank() == 0) cout << "bcasting " << sizes[n] << " ints " << NUM_REPS << " times.\n";
		for (int reps=0; reps < NUM_REPS; reps++){

			if(world.rank() == 0) for (int i=0; i<sizes[n]; i++) buf[i] = 1000000 * (n * NUM_REPS + reps) + i;
			else for (int i=0; i< (int)sizes[n]; i++) buf[i] = -1 - (n * NUM_REPS + reps);

			world.broadcast(buf.begin(), buf.begin() + sizes[n], 0);

			for (int i=0; i<sizes[n]; i++) assert( buf[i] == 1000000 * (n * NUM_REPS + reps) + i );

		}
	}
	return 0;
}

