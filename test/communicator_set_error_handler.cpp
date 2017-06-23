#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wall -Wfatal-errors $0 -o $0x.x && time mpirun -np 4s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"
#include "alf/boost/mpi3/error_handler.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	world.set_error_handler(mpi3::error_handler::code); // default, internal function returns codes
	double d = 5.;
	try{
		world.send(&d, &d + 1, 100);
	}catch(...){
		cout << "catched exception" << std::endl;
	}

	world.set_error_handler(mpi3::error_handler::fatal); // fail immediately 
	world.send(&d, &d + 1, 100);

	return 0;
}

