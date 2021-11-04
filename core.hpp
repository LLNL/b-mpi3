#if COMPILATION// -*- indent-tabs-mode:t;c-basic-offset:4;tab-width:4; -*-
$CXXX `mpicxx -showme:compile|sed 's/-pthread/ /g'` -std=c++14 $0 -o $0x `mpicxx -showme:link|sed 's/-pthread/ /g'`&&mpirun -n 4 $0x&&rm $0x;exit
#endif
// © Alfredo A. Correa 2018-2021

#ifndef BOOST_MPI3_CORE_HPP
#define BOOST_MPI3_CORE_HPP

#include<mpi.h> // if you get a compilation error here it means that 1) you need to compile or defined your CXX as mpic++ or 2) have not setup the compilation flags to find MPI headers, or 3) not installed an MPI implementation (e.g. `apt install libopenmpi-dev openmpi-bin`)

#include<stdexcept>

namespace boost {
namespace mpi3 {

inline bool initialized() {
	int flag;  // NOLINT(cppcoreguidelines-init-variables) delayed init
	int s = MPI_Initialized(&flag);
	if(s != MPI_SUCCESS) {throw std::runtime_error{"cannot probe initialization"};}
	return flag != 0;
}

inline bool finalized() {
	int flag;  // NOLINT(cppcoreguidelines-init-variables) delayed init
	int s = MPI_Finalized(&flag);
	if(s != MPI_SUCCESS) {throw std::runtime_error{"cannot probe finalization"};}
	return flag != 0;
}

}  // end namespace mpi3
}  // end namespace boost

#if not __INCLUDE_LEVEL__ // _TEST_BOOST_MPI3_ENVIRONMENT

#include<cassert>

namespace mpi3 = boost::mpi3;

int main(){
	assert(not mpi3::initialized() );
}

#endif
#endif

