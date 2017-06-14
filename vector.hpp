#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++17 -Wfatal-errors -D_TEST_BOOST_MPI3_VECTOR -lboost_mpi -lboost_serialization -lboost_timer $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_VECTOR_HPP
#define BOOST_MPI3_VECTOR_HPP

#include "alf/boost/version.hpp"
#include<mpi.h>
#include<numeric> // std::accumulate
#include<cassert>

#include "../mpi3/allocator.hpp"
#include<vector>

namespace boost{
namespace mpi3{

template<class T>
using vector = std::vector<T, boost::mpi3::allocator<T>>;

}}

#ifdef _TEST_BOOST_MPI3_VECTOR

#include "alf/boost/mpi3/main.hpp"

using std::cout;
using std::endl;

int boost::mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){

	boost::mpi3::vector<double> v(1000000);

	return 0;
}


#endif
#endif

