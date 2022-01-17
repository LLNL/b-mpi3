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
}

void test_axis(mpi3::cartesian_communicator<3>& comm) {
////  cerr<<"+ I am rank "<< comm.rank() <<" and have coordinates "<< comm.coordinates()[0] <<", "<< comm.coordinates()[1] <<", "<< comm.coordinates()[2] <<'\n';

//	auto comm_sub = comm.sub();
//	static_assert( comm_sub.dimensionality == 2 , "!" );
////  std::cout << "numelements " << comm_sub.num_elements() << std::endl;
//	assert( comm_sub.num_elements() == 4 );

//	assert( comm_sub.shape()[0] == 2 );
//	assert( comm_sub.shape()[1] == 2 );
	{
		auto comm_sub0 = comm.axis(0);
		assert( comm_sub0.shape()[0] == 3 );
		assert( comm_sub0.size() == 3 );
	}
	{
		auto comm_sub1 = comm.axis(1);
		assert( comm_sub1.shape()[0] == 2 );
		assert( comm_sub1.size() == 2 );
	}
	{
		auto comm_sub2 = comm.axis(2);
		assert( comm_sub2.shape()[0] == 2 );
		assert( comm_sub2.size() == 2 );
	}
}

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator /*world*/) -> int try {  // NOLINT(readability-function-cognitive-complexity)

//	assert( world.size() == 12 );
//	mpi3::cartesian_communicator<2> world23(world, {2, 3});

//	mpi3::cartesian_communicator<2> WORLD23(std::move(world23));

//	mpi3::cartesian_communicator<2> empty;

	division_tests1();
	division_tests2();

//{
//	assert( world.size() == 12 );
//	mpi3::cartesian_communicator<2> cart_comm(world, {3, 4});
//	assert( cart_comm );
//	assert( cart_comm.dimensions()[0] == 3 );
//	assert( cart_comm.dimensions()[1] == 4 );

//	auto row = cart_comm.axis(0);
//	auto col = cart_comm.axis(1);
//	assert( row.size() == 3 );
//	assert( col.size() == 4 );
//}

//  vvvv TODO(correaa) this fails for MPICH
//{
//	assert(world.size() == 6);
//	if(mpi3::cartesian_communicator<2> cart_comm{world, {2, 2}}){
//		auto row = cart_comm.axis(0);
//		auto col = cart_comm.axis(1);
//	}
//}

//try {
//	assert(world.size() == 12);
//	mpi3::cartesian_communicator<2> cart_comm(world, {8});
//	assert(cart_comm.dimensions()[0] == 2);
//	assert(cart_comm.dimensions()[1] == 3);
//} catch(...) {}

//{
//	mpi3::cartesian_communicator<2> cart_comm(world, {2, mpi3::fill});
//	assert(cart_comm.dimensions()[0] == 2);
//	assert(cart_comm.dimensions()[1] == 6);
//}
//{
//	mpi3::cartesian_communicator<2> cart_comm(world, {mpi3::fill, 2});
//	assert(cart_comm.dimensions()[0] == 6);
//	assert(cart_comm.dimensions()[1] == 2);
//}

//{
//	assert(world.size() == 12);

//	mpi3::cartesian_communicator<3> comm(world, {});
//	static_assert( mpi3::cartesian_communicator<3>::dimensionality == 3, "!");
//	assert( comm.cartesian_communicator<>::dimensionality() == 3 );

//	assert( comm.num_elements() == world.size() );

//	assert( comm.shape()[0] == 3 );
//	assert( comm.shape()[1] == 2 );
//	assert( comm.shape()[2] == 2 );

//	test_axis(comm);

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

