#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 2 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/window.hpp"

#include<cassert>

namespace mpi3 = boost::mpi3;
using std::cout;

#define NROWS 100
#define NCOLS 100

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	double A[NROWS][NCOLS];

	if(world.root()){
		for(int i = 0; i != NROWS; ++i)
			for(int j = 0; j != NCOLS; ++j)
				A[i][j] = i*NCOLS + j;

		mpi3::window w = world.make_window();
		w.fence(); // note the two fences here
		w.accumulate_n( (double*)A, NROWS*NCOLS, 1 );
		w.fence();
	}else{
		for(int i = 0; i != NROWS; ++i)
			for(int j = 0; j != NCOLS; ++j)
				A[i][j] = i*NCOLS + j;
		mpi3::window w = world.make_window( (double*)A, NROWS*NCOLS );
		w.fence(); // note the two fences here
		w.fence();
		for(int i = 0; i != NROWS; ++i){
			for(int j = 0; j != NCOLS; ++j){
				if(world.rank() == 1) assert( A[i][j] == (i*NCOLS + j)*2 );
				else assert( A[i][j] == i*NCOLS + j );
			}
		}
	}

	return 0;
}

