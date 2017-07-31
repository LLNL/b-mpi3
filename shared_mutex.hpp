#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_SHARED_MUTEX $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHARED_MUTEX_HPP
#define BOOST_MPI3_SHARED_MUTEX_HPP

#include "../mpi3/shared_window.hpp"
#include "../mpi3/detail/basic_mutex.hpp"

namespace boost{
namespace mpi3{

using shared_mutex = detail::basic_mutex<mpi3::shared_window>;

}}

#ifdef _TEST_BOOST_MPI3_SHARED_MUTEX

#include "alf/boost/mpi3/main.hpp"

#include<iostream>
#include<mutex> // lock_guard

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::shared_mutex m(world);
	{
		std::lock_guard<mpi3::shared_mutex> lock(m);
		cout << "locked from " << world.rank() << '\n';
		cout << "never interleaved " << world.rank() << '\n';
		cout << "forever blocked " << world.rank() << '\n';
		cout << std::endl;
	}

}
#endif
#endif

