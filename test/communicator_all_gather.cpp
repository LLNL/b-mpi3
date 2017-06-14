#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 12s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	int max_procs = 10;
	int table[max_procs][max_procs];

	int participants;

	if(world.size() >= max_procs){
		participants = max_procs;
	}else{
		world.abort(1);
	}

	if(world.rank() < participants){
		int block_size = max_procs/participants;
		int begin_row = world.rank()*block_size;
		int end_row = (world.rank() + 1)*block_size;
		int send_count = block_size*max_procs;
		int recv_count = send_count;
		for(int i = begin_row; i != end_row; ++i){
			for(int j = 0; j != max_procs; ++j){
				table[i][j] = world.rank() + 10;
			}
		}
		world.all_gather(
			&table[begin_row][0], &table[begin_row][0] + send_count, 
			&table[0][0]
		); // this is giving an aliasing error, even in the original example
		for(int i = 0; i != max_procs; ++i){
			assert( table[i][0] == table[i][max_procs - 1] );
		}
	}

}

