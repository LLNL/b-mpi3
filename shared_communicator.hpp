#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_SHARED_COMMUNICATOR $0x.cpp -o $0x.x && time mpirun -np 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHARED_COMMUNICATOR_HPP
#define BOOST_MPI3_SHARED_COMMUNICATOR_HPP

#include "../mpi3/communicator.hpp"

namespace boost{
namespace mpi3{

template<class T = void>
struct shared_window;

struct shared_communicator : communicator{
	shared_communicator(communicator const& comm, int key = 0){
		int s = MPI_Comm_split_type(comm.impl_, MPI_COMM_TYPE_SHARED, key,  MPI_INFO_NULL, &impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot split shared");
	}
	template<class T = char>
	shared_window<T> make_shared_window(mpi3::size_t size, int disp_unit = sizeof(T));
	template<class T = char>
	shared_window<T> make_shared_window();
	template<
		class It1, class Size, class Op, 
		class V1 = typename std::iterator_traits<It1>::value_type, 			class P1 = decltype(detail::data(It1{})), 
		class PredefinedOp = predefined_operation<Op>
	>
	auto all_reduce_n(It1 first, Size count, Op op){
		using detail::data;
		int s = MPI_Allreduce(MPI_IN_PLACE, data(first), count, detail::basic_datatype<V1>{}, PredefinedOp{}, impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot reduce n");
	}
};

shared_communicator communicator::split_shared(int key /*= 0*/) const{
	return shared_communicator(*this, key);
}

}}

#ifdef _TEST_BOOST_MPI3_SHARED_COMMUNICATOR

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/operation.hpp"
#include "alf/boost/mpi3/shared_window.hpp"

#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	mpi3::shared_communicator node = world.split_shared();
	mpi3::shared_window win = node.make_shared_window<int>(node.rank()?0:1);
	assert(win.base() != nullptr and win.size<int>() == 1);
	win.lock_all();
	if(node.rank()==0){
		*win.base<int>() = 42;
		win.sync();
	}
	for (int j=1; j != node.size(); ++j) {
	    if (node.rank()==0) node.send_n((int*)nullptr, 0, j, 666);
	    else if (node.rank()==j) node.receive_n((int*)nullptr, 0, 0, 666);
	}
	if(node.rank()!=0){
		win.sync();
	}
	int l = *win.base<int>();
	win.unlock_all();

	int minmax[2] = {-l,l};
	node.all_reduce_n(&minmax[0], 2, mpi3::max<>{});
	assert( -minmax[0] == minmax[1] );
}

#endif
#endif

