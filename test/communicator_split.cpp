#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -n 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/buffer.hpp"
#include "../../mpi3/detail/strided.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	std::vector<int> a(10);
	std::vector<int> b(10);

	int color = world.rank() % 2;

	mpi3::communicator s = world.split(color, world.rank());
/*
	mpi3::communicator c = s.intercommunicator_create(0, world, 1 - color, 52);

	mpi3::scoped_attach<> buffer(2000);

	for(int j=0; j != 10; ++j){
		for(int i=0; i != 10; ++i){
			a[i] = (c.rank() + 10 * j) * c.remote_size() + i;
		}
		c.send(a.begin(), a.end(), 0, 27 + j);
	}
	if(c.rank() == 0){
		for(int i=0; i != c.remote_size(); ++i){
			for(int j=0; j != 10; ++j){
				c.receive(b.begin(), b.end(), i, 27 + j);
				for(int k=0; k != 10; ++k) assert( b[k] == (i + 10*j)*c.remote_size() + k );
			}
		}
	}
*/
	return 0;
}

