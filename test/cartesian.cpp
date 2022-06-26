// Copyright 2021-2022 Alfredo A. Correa

#include "../../mpi3/main.hpp"
#include "../../mpi3/cartesian_communicator.hpp"

namespace mpi3 = boost::mpi3;

void division_tests1() {

{
	auto div = mpi3::cartesian_communicator<2>::division(6);
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 3 );
	assert( div[1] == 2 );
}
{
	auto div = mpi3::cartesian_communicator<2>::division(6, {});
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 3 );
	assert( div[1] == 2 );
}
{
	auto div = mpi3::cartesian_communicator<2>::division(6, {mpi3::fill});
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 3 );
	assert( div[1] == 2 );
}
{
	auto div = mpi3::cartesian_communicator<2>::division(6, {mpi3::fill, mpi3::fill});
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 3 );
	assert( div[1] == 2 );
}
{
	auto div = mpi3::cartesian_communicator<2>::division(6, {2});
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 2 );
	assert( div[1] == 3 );
}
}

void division_tests2() {
{
	auto div = mpi3::cartesian_communicator<2>::division(6, {2, mpi3::fill});
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 2 );
	assert( div[1] == 3 );
}
{
	auto div = mpi3::cartesian_communicator<2>::division(6, {mpi3::fill, 3});
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 2 );
	assert( div[1] == 3 );
}
{
	auto div = mpi3::cartesian_communicator<2>::division(6, {mpi3::_, 3});
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 2 );
	assert( div[1] == 3 );
}
{
	auto div = mpi3::cartesian_communicator<2>::division(7);
	assert( div[0]*div[1] == 7 );
	assert( div[0] == 7 );
	assert( div[1] == 1 );
}
{
	auto div = mpi3::cartesian_communicator<2>::division(7, {mpi3::fill, mpi3::fill});
	assert( div[0]*div[1] == 7 );
	assert( div[0] == 7 );
	assert( div[1] == 1 );
}

try {  // this is an error in MPICH and openMPI
	auto const div = mpi3::cartesian_communicator<2>::division(7, {2, mpi3::fill});
	assert( div[0]*div[1] == 4 );
	assert( div[0] == 2 );
	assert( div[1] == 2 );
} catch(std::runtime_error& e) {
}

try {  // this is an error in MPICH
	auto const div = mpi3::cartesian_communicator<2>::division(6, {2, 2});
	assert( div[0]*div[1] == 4 );
	assert( div[0] == 2 );
	assert( div[1] == 2 );
} catch(std::runtime_error& e) {
}

}

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try {  // NOLINT(readability-function-cognitive-complexity) test function
	assert( world.size() == 6 );

	division_tests1();
	division_tests2();

{
	mpi3::cartesian_communicator<2> cart_comm(world, {3, 2});
	assert( cart_comm.dimensions()[0] == 3 );
	assert( cart_comm.dimensions()[1] == 2 );

	auto row = cart_comm.axis(0);
	auto col = cart_comm.axis(1);
	assert( row.size() == 3 );
	assert( col.size() == 2 );
}
{
	mpi3::cartesian_communicator<2> cart_comm(world, {mpi3::fill, 2});
	assert( cart_comm.dimensions()[0] == 3 );
	assert( cart_comm.dimensions()[1] == 2 );

	auto row = cart_comm.axis(0);
	auto col = cart_comm.axis(1);
	assert( row.size() == 3 );
	assert( col.size() == 2 );
}
{
	mpi3::cartesian_communicator<2> cart_comm(world, {3, mpi3::fill});
	assert( cart_comm.dimensions()[0] == 3 );
	assert( cart_comm.dimensions()[1] == 2 );

	auto row = cart_comm.axis(0);
	auto col = cart_comm.axis(1);
	assert( row.size() == 3 );
	assert( col.size() == 2 );
}

{
	mpi3::cartesian_communicator<2> cart_comm(world, {3, 2});
	assert( cart_comm.dimensions()[0] == 3 );
	assert( cart_comm.dimensions()[1] == 2 );

	auto row = cart_comm.axis(0);
	auto col = cart_comm.axis(1);
	assert( row.size() == 3 );
	assert( col.size() == 2 );

	{
		auto comm_sub0 = cart_comm.axis(0);
		assert( comm_sub0.shape()[0] == 3 );
		assert( comm_sub0.size() == 3 );
	}
	{
		auto comm_sub1 = cart_comm.axis(1);
		assert( comm_sub1.shape()[0] == 2 );
		assert( comm_sub1.size() == 2 );
	}
	{
		auto plane0 = cart_comm.hyperplane(0);
		static_assert( decltype(plane0)::dimensionality == 1 , "!" );
		assert( plane0.size() == 2 );
	}
	{
		auto plane1 = cart_comm.hyperplane(1);
		static_assert( decltype(plane1)::dimensionality == 1 , "!" );
		#if not defined(MPICH_VERSION)
		{  // cartesian communicators of dimension 0 do not work in MPICH
			assert( plane1.size() == 3 );
			auto point = plane1.hyperplane(0);
			assert( point.num_elements() == 1 );
		}
		#endif
	}
}
{
	mpi3::cartesian_communicator<2> cart_comm(world, {3, 2});

	assert( cart_comm.rank() == cart_comm.rank(cart_comm.coordinates()) );
	assert( cart_comm.coordinates() == cart_comm.coordinates(cart_comm.rank()) );

	assert( cart_comm(2, 1).rank() == cart_comm.rank({2, 1}) );
	assert( std::apply(cart_comm, cart_comm.coordinates()).rank() == cart_comm.rank() );

	assert( cart_comm.rank(cart_comm.coordinates(4)) == 4 );

	std::cout<< cart_comm.rank({5, 3}) <<std::endl;


//	switch(world.rank()) {
//		case 1: std::cout<< world.rank() <<" "<< cart_comm.coordinates()[0] <<", "<< cart_comm.coordinates()[1] <<std::endl;
//		case 5: std::cout<< world.rank() <<" "<< cart_comm.coordinates()[0] <<", "<< cart_comm.coordinates()[1] <<std::endl;
//	}
}

	return 0;
} catch(...) {return 1;}
