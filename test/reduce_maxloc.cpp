#include "../../mpi3/main.hpp"

namespace mpi3 = boost::mpi3;

auto mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator world) -> int try {

	int const data = [&] {switch(world.rank()) {
	    case 0: return 12;
	    case 1: return 34;
	    case 2: return 56;
	    case 3: return 78;  // default: world.abort(EXIT_FAILURE);
	} return 0;}();

	{
		auto reduction = world.max_loc(data);

		assert( reduction.first == 78 );
		assert( reduction.second == 3 );
	}
#if 0
	{
		auto&& [m, p] = world.max_location(data);
		assert( m == 78 );
	}
#endif
	{
		int const* max_ptr = world.max_element(data);
		if(world.rank() == 3) {
			assert( max_ptr and max_ptr == &data );
		} else {
			assert( not max_ptr );
		}
	}

	return 0;
} catch(...) {
	return 1;
}
