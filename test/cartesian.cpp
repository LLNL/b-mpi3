// © Alfredo Correa 2021

#include "../../mpi3/main.hpp"
#include "../../mpi3/cartesian_communicator.hpp"

namespace mpi3 = boost::mpi3;

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try{

	mpi3::cartesian_communicator<2> world23(world, {2, 3});

	return 0;
}catch(...){
	return 1;
}

