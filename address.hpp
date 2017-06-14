#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_ADDRESS -lboost_mpi -lboost_serialization -lboost_timer $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_ADDRESS_HPP
#define BOOST_MPI3_ADDRESS_HPP


#include "../mpi3/detail/call.hpp"

#include<mpi.h>

#include<stdexcept>

namespace boost{
namespace mpi3{

using address = MPI_Aint;

address get_address(void const* location){
	address ret = -1;
	int status = MPI_Address(location, &ret);
	if(status != 0) throw std::runtime_error("error taking address");
	return ret;
}

template<class T>
address addressof(T const& t){return get_address(std::addressof(t));}

}}

#ifdef _TEST_BOOST_MPI3_ADDRESS

#include "alf/boost/mpi3/main.hpp"

using std::cout;

int boost::mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){
	std::vector<int> v(10);
	boost::mpi3::address a1 = boost::mpi3::addressof(v[0]);
	boost::mpi3::address a2 = boost::mpi3::addressof(v[1]);
	assert( a2 - a1 == sizeof(int) ); 
}

#endif
#endif

