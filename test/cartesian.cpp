// © Alfredo Correa 2021

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

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try {
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
}


////	{
////		auto plane0 = comm.hyperplane(0);
////		static_assert( decltype(plane0)::dimensionality == 2 , "!" );
////		assert( plane0.num_elements() == 4 );
////		assert( plane0.shape()[0] == 2 );
////		assert( plane0.shape()[1] == 2 );
////	}
////	{
////		auto plane1 = comm.hyperplane(1);
////		static_assert( decltype(plane1)::dimensionality == 2 , "!" );
////		assert( plane1.num_elements() == 6 );
////		assert( plane1.shape()[0] == 3 );
////		assert( plane1.shape()[1] == 2 );
////	}
////	{
////		auto plane2 = comm.hyperplane(2);
////		static_assert( decltype(plane2)::dimensionality == 2 , "!" );
////		assert( plane2.num_elements() == 6 );
////		assert( plane2.shape()[0] == 3 );
////		assert( plane2.shape()[1] == 2 );
////	}
//}
	return 0;
} catch(...) {return 1;}
