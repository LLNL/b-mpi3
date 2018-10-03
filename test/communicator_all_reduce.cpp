#if COMPILATION_INSTRUCTIONS
mpic++ -std=c++14 -O3 -Wall -Wextra `#-Wfatal-errors` $0 -o $0x.x && mpirun -n 8 $0x.x $@ && rm -f $0x.x; exit
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

	auto rank = world.rank();
	auto sum_rank = 0;
	world.all_reduce_n(&rank, 1, &sum_rank);
//	world.all_reduce_n(&rank, 1, &sum_rank, std::plus<>{});
//	world.all_reduce_n(&rank, 1, &sum_rank, mpi3::plus<>{});
//	sum_rank = (world += rank);
	cout << "sum " << sum_rank << std::endl;

	auto max_rank = -1;
	world.all_reduce_n(&rank, 1, &max_rank, mpi3::max<>{});
	cout << "max " << max_rank << std::endl;

	auto min_rank = -1;
	world.all_reduce_n(&rank, 1, &min_rank, mpi3::min<>{});
	cout << "min " << min_rank << std::endl;

	return 0;
}

