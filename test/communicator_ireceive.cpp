#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wall -Wextra `#-Wfatal-errors` -fmax-errors=2 $0 -o $0x.x -lpthread -lboost_serialization && time mpirun -n 2 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

#include<list>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	assert( world.size() == 2);

	using T = std::string;
	std::list<T> const cbuffer = {"0", "1", "2"};//(10); std::iota(buffer.begin(), buffer.end(), 0);
	std::list<T> buffer(3);

	int right = world.right();
	int left = world.left();
	{
		auto req = world.ireceive(buffer.begin(), left);
		world.send(cbuffer.begin(), cbuffer.end(), right);
	} //	req.wait();
	assert( std::equal(cbuffer.begin(), cbuffer.end(), buffer.begin()) );

	return 0;
}

