#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wall -Wextra $0 -o $0x.x && time mpirun -n 8 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/ostream.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	mpi3::ostream wout(world);
	
	mpi3::communicator third = world.divide_even(3); // divide_low // divide_high

	wout << "I am 'world' rank "<<world.rank(); 
	if(third) wout << " and " << third.name() <<"'s rank "<<third.rank() << " with color attribute " << mpi3::any_cast<int>(third.attribute("color"));
	else wout << " and not in 'third'";
	wout << std::endl;

	if(third and mpi3::any_cast<int>(third.attribute("color")) == 0) cout << "I am in leader group " << std::endl;

	return 0;
}

