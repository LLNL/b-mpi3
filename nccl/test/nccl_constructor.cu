#include "../../../mpi3/nccl/communicator.hpp"
#include "../../../mpi3/main.hpp"

#include "/home/correaa/prj/alf/boost/multi/include/multi/array.hpp"
#include <thrust/system/cuda/memory.h>

namespace mpi3 = boost::mpi3;
namespace multi = boost::multi;

int mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator world) {
	assert(world.size() == 4);

//	cudaSetDevice(world_rank); // GPU N binds to MPI rank N

	auto hemi = world / 2;

	mpi3::nccl::communicator magnesium{hemi};

//  multi::array<int64_t, 1, thrust::cuda::universal_allocator<int64_t>> A({1000}, world.rank());
	multi::array<int64_t, 1, thrust::cuda::allocator<int64_t>> A({1000}, world.rank());

	magnesium.all_reduce_n(A.data_elements(), A.num_elements(), A.data_elements());
	multi::array<int64_t, 1> H = A;

	std::cout<<"[rank"<< world.rank() <<"] result:"<< H[0] <<std::endl;

	return 0;
}
