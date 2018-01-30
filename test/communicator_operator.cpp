#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

#include <chrono> //literals
#include <thread> //sleep_for

namespace mpi3 = boost::mpi3;
using std::cout;
using namespace std::chrono_literals;


int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	std::vector<double> inbuf(100);
	std::vector<double> outbuf(100);

	assert(world.size() == 8);
	mpi3::communicator comm = (world <= 1);
	assert(comm.size() == 6 or comm.size() == 2);
	if(world.rank() > 1) return 0;
	assert(comm.size() == 2);

	std::cout << "world[" << world.rank() << "] is also comm[" << comm.rank() << "]" << std::endl;
	mpi3::request r;
	if(world.rank() == 0){
		std::iota(outbuf.begin(), outbuf.end(), 0.0);
		std::this_thread::sleep_for(10s);
		std::cout << "comm[" << comm.rank() << "] about to isent" << std::endl;
		r = comm.isend(outbuf.begin(), outbuf.end(), 1);
		std::cout << "comm[" << comm.rank() << "] isent" << std::endl;
	}else if(world.rank() == 1){
		std::cout << "comm[" << comm.rank() << "] about to ireceive" << std::endl;
		r = comm.ireceive(inbuf.begin(), inbuf.end(), 0);
		std::cout << "comm[" << comm.rank() << "] ireceive" << std::endl;
	}
	r.wait();
	std::cout << "comm[" << comm.rank() << "] completed op" << std::endl;

	if(world.rank() == 1) assert( inbuf[9] == 9. );
	
	

	return 0;
}

