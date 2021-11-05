#if COMPILATION_INSTRUCTIONS
mpic++ $0 -o $0x&&mpirun --oversubscribe -n 8 $0x&&rm $0x;exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"

#include<list>
#include<vector>

namespace mpi3 = boost::mpi3;

struct projector {
	projector() = default;
	explicit projector(mpi3::communicator comm) : comm_{std::move(comm)}, n_{5} {}
	projector(projector&&) = default;
	projector(projector const&) = default;

	auto operator=(projector const&) -> projector& = default;
	auto operator=(projector     &&) -> projector& = default;
//	auto operator=(projector      &) -> projector& = default;

	friend auto operator==(projector const& a, projector const& b) {return a.n_ == b.n_;} //  a.comm_ == b.comm_;}
	friend auto operator!=(projector const& a, projector const& b) {return a.n_ != b.n_;} //  a.comm_ == b.comm_;}

	decltype(auto) get_comm() const {return comm_;}
	auto get_n() const -> int{return n_;}
	~projector() = default;

 private:
	int n_ = 0;
	mutable mpi3::communicator comm_;
};

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try {
//	{
//		projector const p{world};
//		projector p2;
//		p2 = p;
//	}
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
} catch(...) {
	return 0;
}

