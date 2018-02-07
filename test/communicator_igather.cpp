#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 `#-Wfatal-errors` -I$HOME/prj $0 -o $0x.x && time mpirun -np 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"
#include "alf/boost/mpi3/detail/strided.hpp"
#include "alf/boost/mpi3/process.hpp"

#include <thread>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	assert( world.size() > 1);

	{
		std::vector<double> small(10, 5.);
		std::vector<double> large;
		if(world.rank() == 0) large.resize(small.size()*world.size(), -1.);
		{
			auto req = world.igather(small.begin(), small.end(), large.begin(), 0);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(5s);
		}
		if(world.rank() == 0){
			assert( std::all_of(large.begin(), large.end(), [](auto& e){return 5. == e;}) );
		}
	}
	{
		std::vector<int> rs(world.size());
		int r = world.rank();
		world.gather_value(r, rs.begin(), 0);
		if(world.rank() == 0) assert( rs[2] == 2 );
	}
	{
		int r = world.rank() + 1;
		std::vector<int> rs = world.gather_value(r, 0);
		if(world.rank() == 0) assert( rs[2] == 3 );
	}
	{
		int r = world.rank() + 1;
		std::vector<int> rs = (world[0] |= r);
		if(world.rank() == 0) assert( rs[2] == 3 );
	}
	return 0;
}

