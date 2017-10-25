#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 2 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

#include<complex>

namespace mpi3 = boost::mpi3;
using std::cout;


int mpi3::main(int, char*[], mpi3::communicator& world){

	using type = std::complex<double>;
	int N = 10;
	
	assert( world.size() == 2 );

	std::vector<type> buffer(N);
	iota(buffer.begin(), buffer.end(), double{0.});

	if(world.rank()==0){
		std::vector<type> const& cbuffer = buffer;
		world.send(cbuffer.begin(), cbuffer.end(), 1, 123);
	}else{
		std::vector<type> buffer2(N);
		world.receive(buffer2.begin(), buffer2.end(), 0, 123);
	//	world.receive(buffer2.begin());//, 0, 123);
		assert( buffer == buffer2 );
	}
	cout << "finished" << std::endl;
	return 0;

}

