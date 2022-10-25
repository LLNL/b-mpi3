#include "../../mpi3/communicator.hpp"

#include <thrust/system/cuda/memory.h>
//#include <thrust/system/cuda/pointer.h>  // for thrust::cuda::pointer

#include <iostream>

#include <nccl.h>

namespace boost {
namespace mpi3 {
namespace nccl {

namespace detail {

template<class T> struct basic_datatype;

template<> struct basic_datatype<int64_t> : std::integral_constant<ncclDataType_t, ncclInt64 > {};
template<> struct basic_datatype<double > : std::integral_constant<ncclDataType_t, ncclDouble> {};
template<> struct basic_datatype<float  > : std::integral_constant<ncclDataType_t, ncclFloat > {};

template<class T> auto datatype(T const&) -> decltype(basic_datatype<T>::value) {return basic_datatype<T>::value;}

}

struct communicator {
	communicator(mpi3::communicator& mpi) {
		ncclUniqueId nccl_id;
		{
			[[maybe_unused]] auto r = ncclGetUniqueId(&nccl_id);
			assert(r == ncclSuccess);
		}
		mpi.broadcast_n(reinterpret_cast<char*>(&nccl_id), sizeof(ncclUniqueId));
		{
			[[maybe_unused]] ncclResult_t r = ncclCommInitRank(&impl_, mpi.size(), nccl_id, mpi.rank());
			switch(r) {
				case ncclSuccess: break;
				case ncclUnhandledCudaError: assert(0);
				case ncclSystemError: assert(0);
				case ncclInternalError: assert(0);
				case ncclInvalidArgument: assert(0);
				case ncclInvalidUsage: assert(0);
				case ncclRemoteError: assert(0);
				case ncclNumResults: assert(0);
			}
		}
	}
	template<class P1, class Size, class P2, typename = decltype(thrust::raw_pointer_cast(P1{}), thrust::raw_pointer_cast(P2{}))>
	auto all_reduce_n(P1 first, Size count, P2 dest) {
		[[maybe_unused]] ncclResult_t r = ncclAllReduce(
			thrust::raw_pointer_cast(first), thrust::raw_pointer_cast(dest), count, 
			detail::datatype(*raw_pointer_cast(first)), 
			ncclSum, impl_, NULL
		);
//		switch(r) {
//			case ncclSuccess: break;
//			case ncclUnhandledCudaError: assert(0);
//			case ncclSystemError: assert(0);
//			case ncclInternalError: assert(0);
//			case ncclInvalidArgument: assert(0);
//			case ncclInvalidUsage: assert(0);
//			case ncclRemoteError: assert(0);
//			case ncclNumResults: assert(0);
//		}
		return dest + count;
	}

	template<class P, class Size, typename = decltype(thrust::raw_pointer_cast(P{}))>
	auto send_n(P first, Size n, int peer) {
		// ncclGroupStart();
		[[maybe_unused]] ncclResult_t r = ncclSend(thrust::raw_pointer_cast(first), n, detail::datatype(*raw_pointer_cast(first)), peer, impl_, NULL);
		assert( r == ncclSuccess );
		// ncclGroupEnd();
		// cudaStreamSynchronize(NULL);
		return first + n;
	}
	template<class P, class Size, typename = decltype(thrust::raw_pointer_cast(P{}))>
	auto receive_n(P first, Size n, int peer) {
		// ncclGroupStart();
		[[maybe_unused]] ncclResult_t r = ncclRecv(thrust::raw_pointer_cast(first), n, detail::datatype(*raw_pointer_cast(first)), peer, impl_, NULL);
		assert( r == ncclSuccess );
		// ncclGroupEnd();
		// cudaStreamSynchronize(NULL);
		return first + n;
	}
	~communicator() {
		// ncclCommFinalize(impl_);
		ncclCommDestroy(impl_);
	}
	int rank() const {
		int ret;
		[[maybe_unused]] ncclResult_t r = ncclCommUserRank(impl_, &ret);
	//	assert(r == ncclSuccess);
		return ret;
	}
	int count() const {
		int ret;
		[[maybe_unused]] ncclResult_t r = ncclCommCount(impl_, &ret);
		switch(r) {
			case ncclSuccess: break;
			case ncclUnhandledCudaError: assert(0);
			case ncclSystemError: assert(0);
			case ncclInternalError: assert(0);
			case ncclInvalidArgument: assert(0);
			case ncclInvalidUsage: assert(0);
			case ncclRemoteError: assert(0);
			case ncclNumResults: assert(0);
		}
		return ret;
	}

 private:
	ncclComm_t operator&() {return impl_;}
	ncclComm_t impl_;
};

}  // end namespace nccl
}  // end namespace mpi3
}  // end namespace boost
