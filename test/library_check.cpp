// Test for separate compilation / library usage.

#include "../communicator.hpp"
#include "../environment.hpp"

namespace mpi3 = boost::mpi3;

void do_broadcast(mpi3::communicator &c){
	int a = 2;
	c.broadcast_value(a);
}

