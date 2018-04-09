#if COMPILATION_INSTRUCTIONS
mpic++ -std=c++14 -O3 -Wall -Wextra -Wfatal-errors $0 -o $0x.x && mpirun -n 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	assert( world.size() > 1);

	std::vector<std::size_t> local(120);
	iota(begin(local), end(local), 0);

	std::vector<std::size_t> global(local.size(), -1);

	world.all_reduce(begin(local), end(local), begin(global));

	for(std::size_t i = 0; i != global.size(); ++i) 
		assert(global[i] == local[i]*world.size());

	assert( (world += world.rank()) == world.size()*(world.size()-1)/2 );

	return 0;
}

