#include "../../mpi3/communicator.hpp"

namespace boost {
namespace mpi3 {
namespace nccl {

struct communicator {
	communicator(mpi3::communicator& /*other*/) {}
};

}  // end namespace nccl
}  // end namespace mpi3
}  // end namespace boost
