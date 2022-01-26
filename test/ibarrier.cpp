// Copyright 2022 Alfredo A. Correa
#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try {
	auto const my_rank = world.rank();
	mpi3::request r = world.ibarrier();
	std::cout<<"mpi process "<< my_rank <<" call ibarrier."<< std::endl;
	r.wait();
	std::cout<<"mpi process "<< my_rank <<" the barrier is complete."<< std::endl;
	return 0;
} catch(...) {return 1;}
