// #if COMPILATION// -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4-*-
// © Alfredo A. Correa 2018-2021
#ifndef BOOST_MPI3_MAIN_HPP
#define BOOST_MPI3_MAIN_HPP

#include "../mpi3/communicator.hpp"
#include "../mpi3/environment.hpp"
#include "../mpi3/exception.hpp"

namespace boost{
namespace mpi3{

static int main(int /*argc*/, char** /*argv*/, boost::mpi3::communicator /*world*/); // if you include this file you should define `::boost::mpi3::main`NOLINT(bugprone-exception-escape)

}  // end namespace mpi3
}  // end namespace boost

// cppcheck-suppress syntaxError ; bug cppcheck 2.3
auto main(int argc, char** argv) -> int try {  // NOLINT(misc-definitions-in-headers) : if you include this file you shouldn't have your own `::main`, you should define `boost::mpi3::main(int argc, char** argv, boost::mpi3::communicator world)` instead
	boost::mpi3::environment env{argc, argv};
	try{
		return boost::mpi3::main(argc, argv, /*env.*/ boost::mpi3::environment::get_world_instance());
	} catch(std::exception& e) {
		if(boost::mpi3::environment::get_world_instance().root()) {std::cerr<<"exception message: "<< e.what() <<"\n\n\n"<<std::endl;}
		return 1;
	} catch(...) {
		if(boost::mpi3::environment::get_world_instance().root()) {std::cerr<<"unknown exception message"<<std::endl;}
		return 1;
	}
} catch(...) {
	std::cerr<<"unknown error in MPI pogram"<<std::endl;
	return 1;
}


#if not __INCLUDE_LEVEL__ // _TEST_BOOST_MPI3_MAIN

#include "../mpi3/version.hpp"
#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;

int boost::mpi3::main(int argc, char* argv[], mpi3::communicator world) {
	if(world.rank() == 0) cout<< mpi3::version() <<'\n';
	mpi3::communicator duo = world < 2;
	if(duo) cout <<"my rank in comm "<< duo.name() <<" is "<< duo.rank() <<'\n';
	return 0;
}

#endif
#endif


