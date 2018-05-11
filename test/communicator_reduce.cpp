#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wall -Wextra -DOMPI_SKIP_MPICXX `#-Wfatal-errors` $0 -o $0x.x && time mpirun -n 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/detail/strided.hpp"
#include "../../mpi3/process.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	assert( world.size() > 1);
	{
		int count = 120;
		std::vector<int> send_buffer(count);
		iota(send_buffer.begin(), send_buffer.end(), 0);

		std::vector<int> recv_buffer;//(count, -1);
		if(world.rank() == 0) recv_buffer.resize(count, -1);
		world.reduce(send_buffer.begin(), send_buffer.end(), recv_buffer.begin(), std::plus<>{}, 0);
		if(world.rank() == 0)
			for(std::size_t i = 0; i != recv_buffer.size(); ++i) 
				assert(std::size_t(recv_buffer[i]) == i*world.size());
	}
	{
		double v = world.rank();
		double total = world.reduce_value(v, std::plus<>{}, 0);
		if(world.rank() == 0) assert( total == world.size()*(world.size()-1)/2 );
		else assert( total == 0. );
	}
	{
		mpi3::optional<int> total = (world[0] += world.rank());
	//	double total = world.reduce_value(world.rank(), mpi3::sum, 0);
	//	if(total) assert( *total == (world.size()*world.size()-1)/2 );
		if(world.rank() == 0) assert(total);
		if(world.rank() != 0) assert(not total);
		if(total) assert( *total == world.size()*(world.size()-1)/2 );
	}
	return 0;
}

