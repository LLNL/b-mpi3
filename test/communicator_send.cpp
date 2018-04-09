#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -O3 -Wall -Wextra `#-Wfatal-errors` -fmax-errors=2 $0 -o $0x.x -lboost_serialization && time mpirun -n 2 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/communicator.hpp"
#include "../../mpi3/main.hpp"

#include<complex>
#include<string>
#include<list>

namespace mpi3 = boost::mpi3;
using std::cout;

using T = std::string;

int mpi3::main(int, char*[], mpi3::communicator world){

	assert( world.size() == 2 );

	{
		std::list<int> b = {3, 4, 5};
		switch(world.rank()){
			case 0: world.send(cbegin(b), cend(b), 1); break;
			case 1: {
				std::vector<int> b2(b.size());
				world.receive_n(begin(b2), b2.size(), 0);
				std::equal(begin(b), end(b), begin(b2));
			}; break;
		}
	}
	{
		std::vector<std::string> b = {"hola", "blah", "chau"};
		switch(world.rank()){
			case 0: world.send(cbegin(b), cend(b), 1); break;
			case 1: {
				std::list<T> b2(b.size());
				world.receive(begin(b2), 0);
				std::equal(begin(b), end(b), begin(b2));
			}; break;
		}
	}
	return 0;
}

