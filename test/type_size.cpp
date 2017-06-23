#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wall -Wfatal-errors $0 -o $0x.x && time mpirun -np 4s $0x.x $@ && rm -f $0x.x; exit
#endif

#define BOOST_MPI3_DISALLOW_AUTOMATIC_POD_COMMUNICATION

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;


int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	{
		mpi3::type t = mpi3::type::int_;
		assert( t.size() == sizeof(int) );
		assert( t.extent() == sizeof(int) );
		assert( t.lower_bound() == 0 );
		assert( t.upper_bound() == t.extent() - t.lower_bound() );
	}
	{
		mpi3::type t = mpi3::type::float_int;
		assert( t.size() == sizeof(std::pair<float, int>) );
		struct{
			float a; 
			int b;
		} foo;
		assert( t.size() == sizeof(float) + sizeof(int) );
		assert( t.extent() == sizeof(foo) );
		assert( t.lower_bound() == 0 );
		assert( t.upper_bound() == t.extent() - t.lower_bound() );
	}

	return 0;
}

