#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_MAIN $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_MAIN_HPP
#define BOOST_MPI3_MAIN_HPP

#include<mpi.h>

#include "../mpi3/environment.hpp"
#include "../mpi3/communicator.hpp"

namespace boost{
namespace mpi3{

// this definition forces the user to define boost::mpi3::main
int main(int argc, char* argv[], boost::mpi3::communicator& world);

}}

int main(int argc, char* argv[]){
	boost::mpi3::environment env(argc, argv);
	return boost::mpi3::main(argc, argv, env.world());
//	boost::mpi3::environment::initialize(argc, argv);
//	boost::mpi3::communicator::world.name("world");
//	int ret = boost::mpi3::main(argc, argv, boost::mpi3::communicator::world);
//	boost::mpi3::communicator::world.barrier();
//	boost::mpi3::environment::finalize();
}

#ifdef _TEST_BOOST_MPI3_MAIN

#include "../mpi3/version.hpp"
#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){
	if(world.rank() == 0) cout << mpi3::version() << '\n';
	return 0;
}

#endif
#endif

