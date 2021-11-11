// © Alfredo A. Correa 2021
#include "../../mpi3/main.hpp"
#include "../../mpi3/process.hpp"

#include<iostream>
#include<numeric>  // for iota
#include<vector>

namespace bmpi3 = boost::mpi3;

int bmpi3::main(int /*argc*/, char ** /*argv*/, bmpi3::communicator world) try {
	auto const size = world.size(); assert(size != 0);

	int token;  // NOLINT(cppcoreguidelines-init-variables)
	if(world.rank() != 0) {
		world[world.rank() - 1] >> token;
		assert(token == -1);
	} else {
		token = -1;
	}

	world[(world.rank() + 1) %  size] << token;

	if(world.rank() == 0) {
		world[size - 1] >> token;
		assert(token == -1);
	}
	return 0;
} catch(...) {return 1;}
