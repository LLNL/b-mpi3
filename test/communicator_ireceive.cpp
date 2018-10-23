#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wall -Wextra $0 -o $0x.x && mpirun -n 3 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

#include<list>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	assert( world.size() > 1 );

	using T = std::string;
	std::vector<T> const cbuffer = {"0", "1", "2"};//(10); std::iota(buffer.begin(), buffer.end(), 0);
	std::vector<T> buffer(3); // TODO, test with list

	int right = world.right();
	int left = world.left();
	{
		auto req = world.ireceive(buffer.begin(), left);
		world.send(cbuffer.begin(), cbuffer.end(), right);
		cout <<"waiting ireceive in rank "<< world.rank() << std::endl;
		req.wait();
		cout <<"ireceive completed in rank "<< world.rank() << std::endl;
	}
	assert( std::equal(cbuffer.begin(), cbuffer.end(), buffer.begin()) );

	return 0;
}

