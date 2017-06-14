#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` $0 -o $0x.x && time mpirun -np 8s $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/environment.hpp"
//#include "alf/boost/mpi3/communicator.hpp"

using std::cout;
namespace mpi3 = boost::mpi3;

int main(){
#if 0
	mpi3::thread_level provided = mpi3::initialize(mpi3::thread_level::multiple);
	assert( mpi3::is_thread_main() );
	mpi3::thread_level claimed = mpi3::query_thread();
	assert( provided == mpi3::thread_level::multiple );
	assert( claimed == mpi3::thread_level::multiple );
	mpi3::finalize();
#endif
	mpi3::environment env(mpi3::thread_level::multiple);
	assert( env.is_thread_main() );
	assert( env.query_thread() == mpi3::thread_level::multiple );
}

