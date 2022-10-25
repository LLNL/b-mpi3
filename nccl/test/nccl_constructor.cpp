#include "../../../mpi3/nccl/communicator.hpp"
#include "../../../mpi3/main.hpp"

namespace mpi3 = boost::mpi3;

int mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator world) try {
	mpi3::nccl::communicator nickel{world};

	return 0;
} catch(...) {
	return 1;
}
