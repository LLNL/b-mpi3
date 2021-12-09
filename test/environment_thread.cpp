#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 8 $0x.x $@ && rm -f $0x.x; exit
#endif
// Copyright 2018-2021 Alfredo A. Correa
#include "../../mpi3/environment.hpp"

namespace mpi3 = boost::mpi3;

int main() {
	mpi3::environment env{mpi3::thread::serialized};

	assert( env.thread_support() == mpi3::thread::single or env.thread_support() == mpi3::thread::funneled or env.thread_support() == mpi3::thread::serialized );
	assert( env.thread_support() <= mpi3::thread::serialized );
	assert( env.thread_support() <  mpi3::thread::multiple );

	assert( env.is_thread_main() );
}
