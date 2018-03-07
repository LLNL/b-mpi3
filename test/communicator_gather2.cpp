#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wfatal-errors $0 -o $0x.x && time mpirun -n 3 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/detail/iterator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator world){
	using T = std::tuple<double, double>;
	std::vector<T> v_local(10, T{world.rank(), world.rank()});
	std::vector<T> v(world.root()?v_local.size()*world.size():0);
	auto end = world.gather(v_local.begin(), v_local.end(), v.begin());
	if(world.root()) assert(end == v.end());
	if(world.root()){
		assert(v[0] == std::make_tuple(0.,0.));
		assert(v[10] == std::make_tuple(1.,1.));
		assert(v[20] == std::make_tuple(2.,2.));
	}

#if 0
	std::vector<int> v;
	v.resize(v_local.size()*world.size());

	for(int j = 0; j < v_local.size(); j++){
		v_local[j] *= world.rank();
	}

	world.gather_n(v_local.begin(), v_local.size(), v.begin(), 0);
#endif
	return 0;
}

