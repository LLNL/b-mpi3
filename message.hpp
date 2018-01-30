#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_MESSAGE $0x.cpp -o $0x.x && time mpirun -np 8 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_MESSAGE_HPP
#define BOOST_MPI3_MESSAGE_HPP

namespace boost{
namespace mpi3{

class message{
	MPI_Message impl_;
};

}}

#ifdef _TEST_BOOST_MPI3_MESSAGE

#include "alf/boost/mpi3/main.hpp"

namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], mpi3::communicator& world){
	
}

#endif
#endif

