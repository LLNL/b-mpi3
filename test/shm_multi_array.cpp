#if COMPILATION_INSTRUCTIONS
mpic++ -std=c++14 -Wno-deprecated-declarations `#-Wfatal-errors` $0 -o $0x -lboost_serialization&&mpirun -n 4 $0x&&rm $0x;exit
#endif
// © Alfredo A. Correa 2018-2020

#define OMPI_SKIP_MPICXX 1  // https://github.com/open-mpi/ompi/issues/5157

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/process.hpp"
#include "../../mpi3/shm/allocator.hpp"

#include<random>
#include<thread> //sleep_for
#include<mutex> //lock_guard

#include "../../../boost/multi/array.hpp"

namespace mpi3 = boost::mpi3;
namespace multi = boost::multi;

using std::cout;

namespace boost{
namespace mpi3{
namespace shm{

template<class T, boost::multi::dimensionality_type D>
using array = multi::array<T, D, mpi3::shm::allocator<T>>;

}}}

template<class T> void what(T&&) = delete;

int mpi3::main(int, char*[], mpi3::communicator world){

	mpi3::shared_communicator node = world.split_shared();

{
	multi::array<double, 2, mpi3::shm::allocator<double>> A({5, 5}, 1.); // implicitly uses self communicator
	multi::array<double, 2, mpi3::shm::allocator<double>> B({5, 5}, 2.);

	assert( A[1][2] == 1 );
	assert( B[2][1] == 2 );
	
	B = A;

	assert( B[2][1] == 1 );
	assert( B[3][1] == 1 );
}
{
	mpi3::shm::array<double, 2> A({5, 5}, 1., &node);
	mpi3::shm::array<double, 2> B({5, 5}, 2., &node);

	assert( A[1][2] == 1 );
	assert( B[1][2] == 2 );

	B() = A();

	assert( A[1][2] == 1 );
	assert( B[1][2] == 1 );
}
	return 0;
}

