#if COMPILATION_INSTRUCTIONS
mpic++ $0 -o $0x&&mpirun --oversubscribe -n 8 $0x&&rm $0x;exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;

int mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) try{

	assert( world.size() > 1);

	std::vector<std::size_t> local(120);
	iota(begin(local), end(local), 0);

	std::vector<std::size_t> global(local.size());

	auto last = world.all_reduce_n(local.begin(), local.size(), global.begin());
	assert(last == global.end());

	for(std::size_t i = 0; i != global.size(); ++i){
		assert(global[i] == local[i]*world.size());
	}

	auto const sum_of_ranks = (world += world.rank());
	assert( sum_of_ranks == world.size()*(world.size()-1)/2 );

	auto rank = world.rank();
	auto sum_rank = 0;
	world.all_reduce_n(&rank, 1, &sum_rank);
//	world.all_reduce_n(&rank, 1, &sum_rank, std::plus<>{});
//	world.all_reduce_n(&rank, 1, &sum_rank, mpi3::plus<>{});
//	sum_rank = (world += rank);
	assert(sum_rank == world.size()*(world.size()-1)/2);

	auto max_rank = -1;
	world.all_reduce_n(&rank, 1, &max_rank, mpi3::max<>{});
	assert( max_rank == world.size() - 1 );

	auto min_rank = -1;
	world.all_reduce_n(&rank, 1, &min_rank, mpi3::min<>{});
	assert( min_rank == 0 );


	{
		std::vector<int> local(20, 1);
		if(world.rank() == 2){local[1] = 0;}

		std::vector<int> global(local.size());
		world.all_reduce_n(local.begin(), local.size(), global.begin());

		assert(global[0] == world.size());
		assert(global[1] == world.size() - 1);
	}
	{
		std::vector<int> local(20, 1);
		if(world.rank() == 2) local[1] = 9;

		std::vector<int> global(local.size());
		world.all_reduce_n(local.begin(), local.size(), global.begin(), std::logical_and<>{});

		assert(global[0] != 0);
		assert(global[1] == 1);
	}
	{
		int b = 1;
		if(world.rank() == 2){b = 0;}
		int all = (world += b);
		assert( all == world.size() - 1 );
	}
	{
		int b = 1;
		if(world.rank() == 2) b = 0;
		int all = (world &= b);
		assert( all == false );
	}
	{
		bool b = world.rank()==2?false:true;
		bool all = (world &= b);
		assert( all == false );
	}

	return 0;
}catch(...){
	return 1;
}

