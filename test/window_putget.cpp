#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/window.hpp"

#include<cassert>

namespace mpi3 = boost::mpi3;
using std::cout;

#define NROWS 100
#define NCOLS 100

#include<iostream>

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	std::vector<double> inbuf(100);
	std::vector<double> outbuf(100);
	
	std::iota(outbuf.begin(), outbuf.end(), 0.0);ge

	mpi3::communicator comm = (world <= 1);
	if(world.rank() > 1) return 0;

	std::iota(outbuf.begin(), outbuf.end(), 0.0);

	mpi3::window w;
	if(world.rank() == 0) 
		w = mpi3::window(comm);
	else if(world.rank() == 1) 
		w = mpi3::window(inbuf.data(), inbuf.size(), comm);
	w.fence();
	if(world.rank() == 0) w.put_n(outbuf.data(), outbuf.size(), 1);
	w.fence();
	if(world.rank() == 1) assert( inbuf[7] == 7.0 );

	return 0;
}

