#if COMPILATION_INSTRUCTIONS
mpic++ $0 -o $0x&&mpirun --oversubscribe -n 8 $0x&&rm $0x;exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

#include<list>
#include<vector>

namespace mpi3 = boost::mpi3;

struct projector{
	explicit projector(mpi3::communicator& comm) : comm_{comm}{}
private:
	mutable mpi3::communicator comm_;
};

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int{

	{
		std::list<mpi3::communicator> v;
		v.emplace_back(world);
		v.emplace_back(world);
	}
//	{ // doesn't compile, communicator is not copiable
//		std::vector<mpi3::communicator> v = {world, world};
//		v.emplace_back(world);
//		v.emplace_back(world);
//	}
	{
		std::vector<projector> v = {projector{world}, projector{world}};
		v.emplace_back(world);
		v.emplace_back(world);
	}

	return 0;
}

