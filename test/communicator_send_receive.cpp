#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

#include<complex>

namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], mpi3::communicator& world){
	std::vector<double> buffer(10);
	iota(buffer.begin(), buffer.end(), world.rank()); 
 
	auto right = (world.rank() + 1 + world.size()) % world.size();
	auto left  = (world.rank() - 1 + world.size()) % world.size();

	assert(buffer[0] == world.rank()); 
	world.send_receive(buffer.begin(), buffer.end(), left);
	assert(buffer[0] == right);
	return 0;
}

