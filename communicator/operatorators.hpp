#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++17 -Wfatal-errors -D_TEST_BOOST_MPI3_COMMUNICATOR_OPERATORS $0x.cpp -o $0x.x && time mpirun -np 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_COMMUNICATOR_OPERATORS_HPP
#define BOOST_MPI3_COMMUNICATOR_OPERATORS_HPP

#include "../../mpi3/communicator.hpp"

namespace boost{
namespace mpi3{
//	communicator operator/(communicator const& comm, int n){
//		int color = comm.rank()*n/comm.size();
//		return comm.split(color);
//	}
}}

#ifdef _TEST_BOOST_MPI3_COMMUNICATOR_OPERATORS
#include "alf/boost/mpi3/main.hpp"

int boost::mpi3::main(int, char*[], communicator const& world){

	auto s = world/2;
	std::cout << "I am " << world.name() << " " << world.rank() << " and I am " << s.name() << " " << s.rank() << std::endl;
}

#endif
#endif

