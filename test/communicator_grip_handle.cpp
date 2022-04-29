// Copyright 2018-2022 Alfredo A. Correa

#include "../../mpi3/communicator.hpp"

#include<iostream>
#include<numeric>
#include<vector>

namespace bmpi3 = boost::mpi3;

int main(int argc, char** argv) {

	MPI_Init(&argc, &argv);
	{

	bmpi3::communicator& world = bmpi3::grip(MPI_COMM_WORLD);

	if(world.size()%2 == 1) {
	   if(world.is_root()) {std::cerr<<"Must be called with an even number of processes"<<std::endl;}
	   return 1;
	}

	std::vector<double> xsend(10); iota(begin(xsend), end(xsend), 0);
	std::vector<double> xrecv(xsend.size(), -1);

	world.send_receive(cbegin(xsend), cend(xsend), (world.rank()/2)*2 + (world.rank()+1)%2, begin(xrecv));

	assert(xrecv[5] == 5);
	if(world.is_root()) {std::cerr<<"successfully completed"<<std::endl;}

	}

	MPI_Finalize();
}
