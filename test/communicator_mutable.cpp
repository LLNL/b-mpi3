#if COMPILATION_INSTRUCTIONS
mpic++ $0 -o $0x&&mpirun --oversubscribe -n 8 $0x&&rm $0x;exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

#include<list>
#include<vector>

namespace mpi3 = boost::mpi3;

struct projector{
	explicit projector(mpi3::communicator comm) : comm_{std::move(comm)}{}
	friend auto operator==(projector const& a, projector const& b) -> bool{return a.comm_ == b.comm_;}
	auto get_comm() const -> mpi3::communicator&{return comm_;}
private:
	mutable mpi3::communicator comm_;
};

auto mpi3::main(int, char*[], mpi3::communicator world) -> int try{
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
	{ // but this works because the member is mutable
		std::vector<projector> v = {projector{world}, projector{world}};
		v.emplace_back(world);
		v.emplace_back(world);
		v.emplace_back(world);
		v[1] = v[0];
		assert( v[1] == v[0] );
		v[1] = std::move(v[0]);
		assert( v[0] == v[0] );
		assert( v[0].get_comm().is_empty() );
		v[1] = static_cast<projector const&>(v[2]);
		assert( v[2].get_comm() == world );
	}
	{
		mpi3::communicator comm;
		assert( not comm );
		assert( comm.is_empty() );
	}

	return 0;
}catch(...){
	return 0;
}

