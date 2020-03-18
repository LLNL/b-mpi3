#if COMPILATION_INSTRUCTIONS /* -*- indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4;-*- */
mpic++ -D_TEST_BOOST_MPI3_ADDRESS -xc++ $0 -o $0x -lboost_serialization&&mpirun --oversubscribe -n 4 $0x&&rm $0x;exit
#endif
#ifndef BOOST_MPI3_ADDRESS_HPP
#define BOOST_MPI3_ADDRESS_HPP

#include "../mpi3/detail/call.hpp"
#include "../mpi3/types.hpp"

#define OMPI_SKIP_MPICXX 1 // https://github.com/open-mpi/ompi/issues/5157
#include<mpi.h>

#include<stdexcept>

namespace boost{
namespace mpi3{

inline address get_address(void const* location){
	address ret; 
	// this function requires an initialized environment, TODO should be a (static?) member of environment?
	MPI_(Get_address)(location, &ret); // MPI_Address is deprecated
	return ret;
}

template<class T>
address addressof(T const& t){return get_address(std::addressof(t));}

}}

#ifdef _TEST_BOOST_MPI3_ADDRESS

#include "../mpi3/main.hpp"

using std::cout;
namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], mpi3::communicator world){

	std::vector<int> v(10);
	mpi3::address a1 = mpi3::addressof(v[0]);
	mpi3::address a2 = mpi3::addressof(v[1]);
	assert( a2 - a1 == sizeof(int) );

	return 0;
}

#endif
#endif

