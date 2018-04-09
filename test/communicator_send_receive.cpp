#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -O3 -Wall -Wextra `#-Wfatal-errors` $0 -o $0x.x -lboost_serialization && time mpirun -n 4 $0x.x $@ && rm -f $0x.x; exit
#endif

#include<iostream>

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

#include<boost/lexical_cast.hpp>

#include<complex>
#include<list>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	auto right = (world.rank() + 1 + world.size()) % world.size();
	auto left  = (world.rank() - 1 + world.size()) % world.size();
	using boost::lexical_cast;
	{
		using T = double;
		using Container = std::vector<T>;
		Container c(10, lexical_cast<T>(world.rank()));
		world.send_receive_n(c.begin(), c.size(), left, right);
		assert( c.front() == lexical_cast<T>(right) );
	}
	{
		using T = double;
		using Container = std::vector<T>;
		Container c(10, lexical_cast<T>(world.rank()));
		world.send_receive(c.begin(), c.end(), left, right);
		assert( c.front() == lexical_cast<T>(right) );
	}
	{
		using T = double;
		using Container = std::list<T>;
		Container c(10, lexical_cast<T>(world.rank()));
		world.send_receive(c.begin(), c.end(), left, right);
		assert( c.front() == lexical_cast<T>(right) );
	}
	{
		using T = std::string;
		using Container = std::list<T>;
		Container c(10, lexical_cast<T>(world.rank()));
		world.send_receive_n(c.begin(), c.size(), left, right);
		assert( c.front() == lexical_cast<T>(right) );
	}
	{
		int buffer[10]; buffer[5] = world.rank();
		int buffer2[10]; buffer2[5] = -1;
		world.send_receive_n(
			buffer, 10, left, 
			buffer2, 10, right
		);
		assert(buffer2[5] == right);
	}
	{
		std::vector<int> buffer(10);  buffer[5] = world.rank();
		std::vector<int> buffer2(10); buffer2[5] = -1;
		world.send_receive(
			buffer.begin(), buffer.end(), left, 
			buffer2.begin(), buffer2.end(), right
		);
		assert(buffer2[5] == right);
	}
	{
		using T = std::complex<double>;
		std::list<T> b(10, T{});//std::to_string(1.*world.rank()));
		world.send_receive_n(b.begin(), b.size(), left, right);
	//	assert( *b.begin() == std::to_string(1.*right) );
	}
	return 0;
}

