#if COMPILATION_INSTRUCTIONS
mpic++ -std=c++14 -O3 -Wall -Wextra -Wfatal-errors $0 -o $0x.x && time mpirun -n 12 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	int chunk = 128;
	std::vector<int> sendbuf(world.size()*chunk);
	std::vector<int> recvbuf(world.size()*chunk);

	for(int i = 0; i != world.size()*chunk; ++i){
		sendbuf[i] = world.rank() + 1;
		recvbuf[i] = 0;
	}
	world.all_to_all(sendbuf.begin(), sendbuf.begin() + chunk, recvbuf.begin());

}

