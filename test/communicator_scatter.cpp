#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 8s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/environment.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int main(int argc, char* argv[]){
	mpi3::environment env;
	mpi3::communicator& world = env.world();

	int table[max_procs][max_procs];
	int row[max_procs];

	int participants = max_procs;
	assert(world.size() == 8);

	if(world.rank() == 0)
		for(int i = 0; i != participants; ++i)
			for(int j = 0; j != participants; ++j)
				table[i][j] = i + j;

//	world.scatter(&table[0][0], &table[0][0] + 64, &row[0], 0);
	world.scatter_from(&row[0], &row[0] + 8, &table[0][0], 0);

	for(int i = 0; i != max_procs; ++i) assert(row[i] == i + world.rank());

}

