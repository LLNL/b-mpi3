#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -n 4 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/window.hpp"

#include<cassert>
#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;


int mpi3::main(int, char*[], mpi3::communicator world){
	mpi3::communicator comm = (world < 2);
	if(comm){
		std::vector<double> inbuf(100);
		std::vector<double> outbuf(100);
		
		std::iota(outbuf.begin(), outbuf.end(), 0.0);
		mpi3::window<double> win{comm, comm.rank()==1?inbuf.data():nullptr, comm.rank()==1?inbuf.size():0};
		win.fence();
		if(world.rank() == 0) win.put_n(outbuf.data(), outbuf.size(), 1);
		win.fence();
		if(world.rank() == 1) assert( inbuf[7] == 7.0 );
	}

	return 0;
}

