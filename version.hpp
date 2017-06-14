#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_VERSION -lboost_mpi -lboost_serialization -lboost_timer $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp $0x.x; exit
#endif
#ifndef BOOST_MPI3_VERSION_HPP
#define BOOST_MPI3_VERSION_HPP

#include<mpi.h>
#include<iostream>

namespace boost{
namespace mpi3{

struct version_t{
	int major;
	int minor;
	friend std::ostream& operator<<(std::ostream& os, version_t const& self){
		return os << self.major << '.' << self.minor;
	}
};

version_t version(){
	version_t ret;
	MPI_Get_version(&ret.major, &ret.minor);
	return ret;
}

}}

#ifdef _TEST_BOOST_MPI3_VERSION

#include "alf/boost/mpi3/main.hpp"

int boost::mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){
	if(world.rank() == 0){
		std::cout 
			<< "size " << world.size() << '\n'
			<< "mpi version " << boost::mpi3::version() << '\n'
		;
	}
}

#endif
#endif

