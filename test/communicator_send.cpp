#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 2s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	assert( world.size() == 2);

	std::vector<int> buffer(10);
	std::iota(buffer.begin(), buffer.end(), 0);

	if(world.root()){
		std::vector<int> const& cbuffer = buffer;
		world.send(cbuffer.begin(), cbuffer.end(), 1, 123);
	}else{
		std::vector<int> buffer2(10);
	//	world.receive(buffer2.begin(), buffer2.end(), 0, 123);
		world.receive(buffer2.begin(), 0, 123);
		assert( buffer == buffer2 );
	}

}

