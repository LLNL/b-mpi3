#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -I${HOME}/prj/ -Wfatal-errors $0 -o $0x.x && time mpirun -np 5 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/environment.hpp"
#include "alf/boost/mpi3/shared_window.hpp"

#include<iostream>

using std::cout;
using std::endl;

namespace mpi3 = boost::mpi3;

int main(int argc, char* argv[]){
	mpi3::environment env(argc, argv);
	auto world = env.world();
	mpi3::shared_communicator node = world.split_shared();
	cout<<" rank:  " <<world.rank() <<endl;

	mpi3::intranode::allocator<double>   A1(node);
	mpi3::intranode::allocator<float>    A2(node);
	mpi3::intranode::allocator<int>      A3(node);
	mpi3::intranode::allocator<unsigned> A4(node);
	mpi3::intranode::allocator<double>   A5(node);
	mpi3::intranode::allocator<float>    A6(node);
	mpi3::intranode::allocator<int>      A7(node);
	mpi3::intranode::allocator<unsigned> A8(node);
	mpi3::intranode::allocator<double>   A9(node);

	auto data1 = A1.allocate(80);
	auto data2 = A2.allocate(80);
	auto data3 = A3.allocate(80);
	auto data4 = A4.allocate(80);
	auto data5 = A5.allocate(80);
	auto data6 = A6.allocate(80);
	auto data7 = A7.allocate(80);
	auto data8 = A8.allocate(80);
	auto data9 = A9.allocate(80);

	using ptr = decltype(data1);
	std::pointer_traits<ptr>::element_type dd = 5.6;
	std::pointer_traits<ptr>::pointer pp = data1;
	double* dppp = std::pointer_traits<ptr>::to_address(data1);

	assert(dd == 5.6);

	if(node.root()) data9[3] = 3.4;
	node.barrier();
	assert(data9[3] == 3.4);
	node.barrier();
	A1.deallocate(data1, 80);
	A2.deallocate(data2, 80);
	A3.deallocate(data3, 80);
	A4.deallocate(data4, 80);
	A5.deallocate(data5, 80);
	A6.deallocate(data6, 80);
	A7.deallocate(data7, 80);
	A8.deallocate(data8, 80);
	A9.deallocate(data9, 80);

}
