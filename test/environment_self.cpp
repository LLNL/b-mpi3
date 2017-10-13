#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/environment.hpp"

using std::cout;
namespace mpi3 = boost::mpi3;

int main(){
	mpi3::environment env;
	auto self = env.self();
	assert( self.size() == 1 );
	assert( self.rank() == 0 );
	cout << "I am process " << self.rank() << " in communicator " << self.name() << std::endl;
}

