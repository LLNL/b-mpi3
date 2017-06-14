#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wfatal-errors -Wunused-result $0 -o $0x.x && time mpirun -np 2s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	assert( world.size() == 2);

	int right = (world.rank() + 1) % world.size();
	int left = world.rank() - 1;
	if(left < 0) left = world.size() - 1;

	std::vector<int> buffer(10); std::iota(buffer.begin(), buffer.end(), 0);
	std::vector<int> buffer2(10);
	mpi3::request r1 = world.ireceive(buffer2.begin(), buffer2.end(), left, 123);
	mpi3::request r2 = world.isend(buffer.begin(), buffer.end(), right, 123);
	r1.wait();
	assert( buffer == buffer2 );
	r2.wait();
}

