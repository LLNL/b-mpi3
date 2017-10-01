#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_VECTOR $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_VECTOR_HPP
#define BOOST_MPI3_VECTOR_HPP

#include "../mpi3/allocator.hpp"

#include<mpi.h>

#include<vector>

namespace boost{
namespace mpi3{

template<class T>
using vector = std::vector<T, mpi3::allocator<T>>;

template<class T>
using uvector = std::vector<T, mpi3::uallocator<T>>;

}}

#ifdef _TEST_BOOST_MPI3_VECTOR

#include "alf/boost/mpi3/main.hpp"

using std::cout;
namespace mpi3 = boost::mpi3;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::vector<long long> v(100);
	std::iota(v.begin(), v.end(), 0);
	assert( std::accumulate(v.begin(), v.end(), 0) == (v.size()*(v.size() - 1))/2 );

	mpi3::vector<std::size_t> uv(100);
	assert( std::accumulate(uv.begin(), uv.end(), 0) == 0 );

	return 0;
}

#endif
#endif

