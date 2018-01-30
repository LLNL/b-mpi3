#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	std::vector<int> v_local = {1,2,3};
	std::vector<int> v;
	v.resize(v_local.size()*world.size());

	for(int j = 0; j < v_local.size(); j++){
		v_local[j] *= world.rank();
	}

	world.gather(v_local.begin(), v_local.end(), v.begin(), 0);
	return 0;
}

