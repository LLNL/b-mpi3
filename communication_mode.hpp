//  (C) Copyright Alfredo A. Correa 2018.
#ifndef MPI3_DETAIL_COMMUNICATION_MODE
#define MPI3_DETAIL_COMMUNICATION_MODE

// #define OMPI_SKIP_MPICXX 1//https://github.com/open-mpi/ompi/issues/5157
#include <mpi.h> // if you get an error here probably you need to compile with and MPI compiler wrapper

#include<utility> // forward

namespace boost{
namespace mpi3{

struct blocking_mode{};
struct nonblocking_mode{};

struct standard_communication_mode {
	template<class... As> static int  Send(As... as) {return MPI_Send  (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int  Recv(As... as) {return MPI_Recv  (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int ISend(As... as) {return MPI_Isend (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int IRecv(As... as) {return MPI_Irecv (as...);}  // NOLINT(readability-identifier-naming)
};

struct buffered_communication_mode {
	template<class... As> static int  Send(As... as) {return MPI_Bsend (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int  Recv(As... as) {return MPI_Brecv (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int ISend(As... as) {return MPI_Ibsend(as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int IRecv(As... as) {return MPI_Ibrecv(as...);}  // NOLINT(readability-identifier-naming)
};

struct synchronous_communication_mode {
	template<class... As> static int  Send(As... as) {return MPI_Ssend (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int  Recv(As... as) {return MPI_Srecv (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int ISend(As... as) {return MPI_Issend(as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int IRecv(As... as) {return MPI_Isrecv(as...);}  // NOLINT(readability-identifier-naming)
};

struct ready_communication_mode {
	template<class... As> static int  Send(As... as) {return MPI_Rsend (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int  Recv(As... as) {return MPI_Rrecv (as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int ISend(As... as) {return MPI_Irsend(as...);}  // NOLINT(readability-identifier-naming)
	template<class... As> static int IRecv(As... as) {return MPI_Irrecv(as...);}  // NOLINT(readability-identifier-naming)
};

struct  gather_mode {template<class... As> int operator()(As... as) const {return MPI_Gather (as...);}};
struct igather_mode {template<class... As> int operator()(As... as) const {return MPI_Igather(as...);}};

struct all_gather_mode {template<class... As> int operator()(As... as) const {return MPI_Allgather(as...);}};

struct  reduce_mode {template<class... As> int operator()(As... as) const {return MPI_Reduce (as...);}};
struct ireduce_mode {template<class... As> int operator()(As... as) const {return MPI_Ireduce(as...);}};

struct all_reduce_mode {template<class... As> int operator()(As... as) const {return MPI_Allreduce(as...);}};

}  // end namespace mpi3
}  // end namespace boost
#endif
