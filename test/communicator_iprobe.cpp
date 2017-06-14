#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wall -Wfatal-errors $0 -o $0x.x && time mpirun -np 4s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	int send_message = 123;
	int receive_message = 0;

	if(world.rank() == 0){
		mpi3::request r = world.isend(&send_message, &send_message + 1, 0, 0);
		while(not world.iprobe(0, 0) ){};
		assert( world.iprobe(0, 0)->count<int>() );
		world.receive(&receive_message, &receive_message + 1, 0, 0);
		assert( receive_message == send_message );
	//	r.wait(); // wait is called on the desctructor now
	}

	return 0;
}

