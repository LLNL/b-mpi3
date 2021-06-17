#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wall -Wextra `#-Wfatal-errors` $0 -o $0x.x && time mpirun -n 8 $0x.x $@ && rm -f $0x.x; exit
#endif
// © Alfredo Correa 2018-2021

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/process.hpp"

namespace mpi3 = boost::mpi3;

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try{
	assert( world.size() > 1);
	
	{
		int count = 120;
		std::vector<int> send_buffer(count);
		iota(send_buffer.begin(), send_buffer.end(), 0);

		std::vector<int> recv_buffer;//(count, -1);
		if(world.rank() == 0){recv_buffer.resize(count, -1);}
		world.reduce_n(send_buffer.begin(), send_buffer.size(), recv_buffer.begin(), std::plus<>{}, 0);
		if(world.rank() == 0){
			for(std::size_t i = 0; i != recv_buffer.size(); ++i){
				assert(std::size_t(recv_buffer[i]) == i*world.size());
			}
		}
	}
	{
		double const v = world.rank();
		double total = 0;
		double const* const it = world.reduce_n(&v, 1, &total, std::plus<>{}, 0);
		if(world.rank() == 0){
			assert( total == static_cast<double>(world.size()*(world.size()-1))/2 );
		}else{
			assert( total == 0 );
		}
		if(world.rank() == 0){
			assert(it != &total);
		}else{
			assert(it == &total);
		}
	}
	{
		mpi3::optional<int> total = (world[0] += world.rank());
		if(world.rank() == 0){assert(total);}
		if(world.rank() != 0){assert(not total);}
		if(total){assert( *total == static_cast<double>(world.size()*(world.size()-1))/2 );}
	}

	return 0;
}catch(...){throw;}

