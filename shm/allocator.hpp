#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && mpic++ -O3 -std=c++14 -Wall -Wfatal-errors -D_TEST_MPI3_SHM_ALLOCATOR $0x.cpp -o $0x.x && time mpirun -n 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef MPI3_SHM_ALLOCATOR_HPP
#define MPI3_SHM_ALLOCATOR_HPP

#include "../../mpi3/shared_window.hpp"

namespace boost{
namespace mpi3{
namespace shm{

template<class... As>
using allocator = mpi3::intranode::allocator<As...>;
template<class T>
using pointer = mpi3::intranode::array_ptr<T>;

template<class Ptr>
struct pointer_traits : std::pointer_traits<Ptr>{
	static auto to_address(Ptr const& p){
		return std::addressof(*p);
	}
};

}}
}

#ifdef _TEST_MPI3_SHM_ALLOCATOR

#include "../../mpi3/main.hpp"
#include "../../mpi3/shm/allocator.hpp"

namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], mpi3::communicator world){

	mpi3::shared_communicator node = world.split_shared();

	mpi3::shm::allocator<double> A1(node);
	mpi3::shm::pointer<double> data1 = A1.allocate(80);

	using ptr = decltype(data1);
	std::pointer_traits<ptr>::pointer pp = data1;
	double* dp = std::addressof(*data1);
	double* dp2 = mpi3::shm::pointer_traits<ptr>::to_address(data1);

	if(node.root()) data1[3] = 3.4;
	node.barrier();
	assert( *dp == *data1 );
	assert( *dp2 == *data1 );
	assert(data1);
	assert(!!data1);
	assert(not (data1 < data1));
	assert(data1[3] == 3.4);

	A1.deallocate(data1, 80);

	return 0;
}

#endif
#endif


