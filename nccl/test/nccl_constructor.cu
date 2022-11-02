#include "../../../mpi3/nccl/communicator.hpp"
#include "../../../mpi3/main.hpp"

#include <thrust/system/cuda/memory.h>
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>

#include <thrust/complex.h>

namespace mpi3 = boost::mpi3;

int mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator WORLD) {
	assert(WORLD.size() == 4);  

//	cudaSetDevice(world_rank); // GPU N binds to MPI rank N

	auto HEMI = WORLD < 2;

	mpi3::nccl::communicator magnesium{HEMI};
	assert(magnesium.rank() == HEMI.rank());

	using T = thrust::complex<double>;  // int64_t;
//  thust::device_vector<T, thrust::cuda::universal_allocator<T>> A(1000, world.rank());
	thrust::device_vector<T, thrust::cuda::allocator<T>> A(1000, T{1.*WORLD.rank()});

	magnesium.all_reduce_n(A.data(), A.size(), A.data());
	thrust::host_vector<T> H = A;

	std::cout<<"[WORLD rank"<< WORLD.rank() <<" HEMI rank"<< HEMI.rank() <<"] result:"<< H[0] <<std::endl;

//  assert( magnesium.count() == 2 );

//  auto magnesium2 = std::move(magnesium);
//  assert( magnesium2.count() == 2 );

	switch(magnesium.rank()) {
	case 0: {
		magnesium.send_n(A.data(), A.size(), 1);
	}
	case 1: {
		thrust::device_vector<T, thrust::cuda::allocator<T>> B(1000, T{});
		magnesium.receive_n(B.data(), B.size(), 0);
		assert( A == B );
	}
	}

	int i = 0;
	if(not magnesium.rank()) { i = 99; }

	magnesium.broadcast_n(&i, 1);
	assert( i == 99 );

	return 0;
}
