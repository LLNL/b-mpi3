#include "../../../mpi3/nccl/communicator.hpp"
#include "../../../mpi3/main.hpp"

#include <thrust/system/cuda/memory.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

namespace mpi3 = boost::mpi3;

int mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator world) {
	assert(world.size() == 4);

//	cudaSetDevice(world_rank); // GPU N binds to MPI rank N

	auto hemi = world / 2;

	mpi3::nccl::communicator magnesium{hemi};

//  thust::device_vector<int64_t, thrust::cuda::universal_allocator<int64_t>> A(1000, world.rank());
	thrust::device_vector<int64_t, thrust::cuda::allocator<int64_t>> A(1000, world.rank());

	magnesium.all_reduce_n(A.data(), A.size(), A.data());
	thrust::host_vector<int64_t> H = A;

	std::cout<<"[rank"<< world.rank() <<"] result:"<< H[0] <<std::endl;

	return 0;
}
