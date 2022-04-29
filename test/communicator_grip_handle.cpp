// Copyright 2018-2022 Alfredo A. Correa

#include "../../mpi3/communicator.hpp"

#include<iostream>
#include<numeric>
#include<vector>

namespace bmpi3 = boost::mpi3;

int main(int argc, char** argv) {

	MPI_Init(&argc, &argv);
	{

	bmpi3::communicator& w = bmpi3::grip(MPI_COMM_WORLD);

	bmpi3::communicator world = w.duplicate();
	world.set_name("sasasa");

	if( w.size()%2 == 0 ) {
		if(w.is_root()) {std::cerr<<"Must be called with an even number of processes"<<std::endl;}
		MPI_Finalize();
		return 1;
	}

	std::vector<double> const xsend(10, 5.);
	std::vector<double>       xrecv(10, -1.);

//  w.send_receive(cbegin(xsend), cend(xsend), (w.rank()/2)*2 + (w.rank()+1)%2, begin(xrecv));

//	assert(xrecv[5] == 5);
	if(w.is_root()) {std::cerr<<"successfully completed"<<std::endl;}
	w.barrier();
	}
	std::cerr<< "finalize" <<std::endl;
	MPI_Finalize();
}
