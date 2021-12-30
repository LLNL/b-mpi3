#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try {

	assert( world.size() > 1);

	{
		std::vector<double> small(10, 5.); iota(begin(small), end(small), world.rank());
		std::vector<double> large;
		if(world.rank() == 0){large.resize(small.size()*world.size(), -1.);}
		{
			auto req = world.igather_n(small.begin(), small.size(), large.begin(), 0);
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(2s);
		//	req.wait();
		}
		if(world.rank() == 0){
			assert( equal(begin(large), begin(large) + small.size(), begin(small)) );
		}
	}
	{
		std::vector<double> small(10, 5.);
		std::vector<double> large(small.size()*world.size(), -1.);
		{
			auto req = world.iall_gather_n(small.begin(), small.size(), large.begin());
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(5s);
		}
		assert( std::all_of(large.begin(), large.end(), [](auto& e){return 5. == e;}) );
	}

	return 0;
} catch(...) {return 1;}
