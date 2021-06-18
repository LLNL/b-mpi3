#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -O3 -Wall -Wextra `#-Wfatal-errors` $0 -o $0x.x -lboost_serialization && time mpirun -n 1 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"

namespace mpi3 = boost::mpi3;

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try{

	assert( not world.is_empty() );
	auto const s = world.size();
	assert( s != 0 );

	auto right = (world.rank() + 1 + s ) % s;
	auto left  = (world.rank() - 1 + s ) % s;

	{
		using Container = std::vector<double>;
		Container c(10, world.rank());
		world.send_receive_n(c.begin(), c.size(), left, right);
		assert( c.front() == right );
	}
	{
		using Container = std::vector<double>;
		Container c(10, world.rank());
		world.send_receive(c.begin(), c.end(), left, right);
		assert( c.front() == right );
	}
	{
		using Container = std::list<double>;
		Container c(10, world.rank());
		world.send_receive(c.begin(), c.end(), left, right);
		assert( c.front() == right );
	}
	{
		using Container = std::list<std::string>;
		Container c(10, std::to_string(world.rank()));
		world.send_receive_n(c.begin(), c.size(), left, right);
		assert( c.front() == std::to_string(right) );
	}
	{
		std::array<int, 10> buffer {}; buffer [5] = world.rank();
		std::array<int, 10> buffer2{}; buffer2[5] = -1;
		world.send_receive_n(
			buffer .data(), 10, left , 
			buffer2.data(), 10, right
		);
		assert(buffer2[5] == right);
	}
	{
		std::vector<int> buffer(10);  buffer[5] = world.rank();
		std::vector<int> buffer2(10); buffer2[5] = -1;
		world.send_receive(
			buffer .begin(), buffer .end(), left , 
			buffer2.begin(), buffer2.end(), right
		);
		assert(buffer2[5] == right);
	}
	{
		std::list<std::complex<double>> b(10, std::complex<double>{});//std::to_string(1.*world.rank()));
		world.send_receive_n(b.begin(), b.size(), left, right);
	//	assert( *b.begin() == std::to_string(1.*right) );
	}
	return 0;
}catch(...){
	return 1;
}

