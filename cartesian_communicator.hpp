#if COMPILATION_INSTRUCTIONS /* -*- indent-tabs-mode: t -*- */
(echo '#include "'$0'" '>$0.cpp)&&mpic++ -std=c++17 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_CARTESIAN_COMMUNICATOR $0.cpp -o $0x&&mpirun -n 12 --oversubscribe $0x&&rm $0x $0.cpp;exit
#endif
// © Alfredo A. Correa 2018-2020

#ifndef BOOST_MPI3_CARTESIAN_COMMUNICATOR_HPP
#define BOOST_MPI3_CARTESIAN_COMMUNICATOR_HPP

#include "../mpi3/communicator.hpp"
#include "../mpi3/process.hpp"

#include "../mpi3/detail/call.hpp"

namespace boost{
namespace mpi3{

using dimensionality_type = int;
static constexpr int dynamic_extent = -1;

template<dimensionality_type D = dynamic_extent> struct cartesian_communicator;

template<>
struct cartesian_communicator<dynamic_extent> : communicator{
	private:
	cartesian_communicator() : communicator(){}
	public:
	template<class Shape, class Period>
	cartesian_communicator(communicator const& comm_old, Shape const& s, Period const& p){
		assert(s.size() == p.size());
		MPI_(Cart_create)(&comm_old, s.size(), s.data(), p.data(), false, &impl_);
		assert(impl_ != MPI_COMM_NULL);
		// there is an bug in mpich, in which if the remaining dim are none then the communicator is not well defined.
	}
	template<class Shape>
	cartesian_communicator(communicator const& comm_old, Shape const& s) : cartesian_communicator(comm_old, s, std::vector<int>(s.size(), true)){}
	
	cartesian_communicator(communicator const& comm_old, std::initializer_list<int> shape) 
		: cartesian_communicator(comm_old, std::vector<int>(shape)){}
	cartesian_communicator(communicator const& comm_old, std::initializer_list<int> shape, std::initializer_list<int> period) 
		: cartesian_communicator(comm_old, std::vector<int>(shape), std::vector<int>(period)){}

	[[deprecated("use dimensionality() instead of dimension")]] 
	int dimension() const{int ret; MPI_Cartdim_get(impl_, &ret); return ret;}

	int dimensionality() const{int ret; MPI_(Cartdim_get)(impl_, &ret); return ret;}
	std::vector<int> coordinates() const{
		std::vector<int> ret(dimensionality());
		MPI_Cart_coords(impl_, rank(), dimensionality(), ret.data());
		return ret;
	}
	auto topology() const{
		auto maxdims = dimensionality();
		struct topology_t{
			std::vector<int> dimensions;
			std::vector<int> periods;
			std::vector<int> coordinates;
			topology_t(std::size_t n) : dimensions(n), periods(n), coordinates(n){}
		} ret(maxdims);
		MPI_(Cart_get)(impl_, maxdims, ret.dimensions.data(), ret.periods.data(), ret.coordinates.data());
		assert( ret.coordinates == coordinates() );
		return ret;
	}
	std::vector<int> shape() const{return topology().dimensions;}
	std::vector<bool> periods() const{auto ps = topology().periods; return {ps.begin(), ps.end()};}
	auto num_elements() const{return size();}

	template<class Coord>
	auto operator()(Coord const& coord){
		int rank = -1;
		MPI_Cart_rank(impl_, coord.data(), &rank);
		return (*this)[rank];
	//	return operator[](rank);
	}
	// int MPI_Cart_map not implemented
	cartesian_communicator sub(std::vector<int> const& remain_dims) const{
		assert( remain_dims.size() == dimensionality() );
		cartesian_communicator ret; MPI_(Cart_sub)(impl_, remain_dims.data(), &ret.impl_); return ret;
	}
	template<class RemainDim = std::initializer_list<bool>>
	cartesian_communicator sub(RemainDim const& remain_dims) const{
		return sub(std::vector<int>(remain_dims.begin(), remain_dims.end()));
	}
	cartesian_communicator sub() const{
		assert( dimensionality()>1 );
		std::vector<int> remain(dimensionality(), true); remain[0] = false;
		return sub(remain);
	}
};

template<dimensionality_type D>
struct cartesian_communicator : cartesian_communicator<>{
	constexpr static dimensionality_type dimensionality = D;
	using cartesian_communicator<dynamic_extent>::cartesian_communicator;
	static auto dims_create(int nnodes, int ndims){
		std::vector<int> dims(ndims, 0);
		MPI_Dims_create(nnodes, ndims, dims.data());
		return dims;
	}
	explicit cartesian_communicator(communicator& other) : 
		cartesian_communicator<>(other, dims_create(other.size(), D))
	{}
	cartesian_communicator<D-1> sub() const{
		auto sh = this->shape();
		auto comm_sub = cartesian_communicator<>::sub();
		return cartesian_communicator<D-1>(comm_sub, comm_sub.shape());
	}
};

#ifdef __cpp_deduction_guides
template<class T> cartesian_communicator(T, std::initializer_list<int>, std::initializer_list<bool>)
	->cartesian_communicator<dynamic_extent>;
template<class T> cartesian_communicator(T, std::initializer_list<int>, std::initializer_list<int>)
	->cartesian_communicator<dynamic_extent>;
template<class T> cartesian_communicator(T, std::initializer_list<int>)
	->cartesian_communicator<dynamic_extent>;
template<class... As> cartesian_communicator(As...)
	->cartesian_communicator<dynamic_extent>;
#endif

}}

#ifdef _TEST_BOOST_MPI3_CARTESIAN_COMMUNICATOR

#include<iostream>
#include "../mpi3/main.hpp"
#include "../mpi3/version.hpp"
#include "../mpi3/ostream.hpp"

using std::cout;
using std::cerr;
namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], boost::mpi3::communicator world){
{
	assert(world.size() == 12);

	mpi3::cartesian_communicator<> comm(world, {4, 3}, {true, false});
	assert( comm.dimensionality() == 2 );
	cerr <<"= I am rank "<< comm.rank() <<" and have coordinates "<< comm.coordinates()[0] <<", "<< comm.coordinates()[1] <<"\n";
	auto comm_sub = comm.sub();
	assert( comm_sub.dimensionality() == 1 );
}
	if(world.root()) cerr<<"---"<<std::endl;
#ifdef __cpp_deduction_guides
{
	assert(world.size() == 12);

	mpi3::cartesian_communicator comm(world, {4, 3}, {true, false});
	assert( comm.dimensionality() == 2 );
	cerr <<"- I am rank "<< comm.rank() <<" and have coordinates "<< comm.coordinates()[0] <<", "<< comm.coordinates()[1] <<"\n";
}
#endif
	if(world.root()) cerr<<"---"<<std::endl;
{
	assert(world.size() == 12);

	mpi3::cartesian_communicator<3> comm{world};
	static_assert( mpi3::cartesian_communicator<3>::dimensionality == 3, "!");
	assert( comm.cartesian_communicator<>::dimensionality() == 3 );
	assert( comm.num_elements() == world.size() );
	assert( comm.shape()[0] == 3 );
	assert( comm.shape()[1] == 2 );
	assert( comm.shape()[2] == 2 );
	cerr<<"+ I am rank "<< comm.rank() <<" and have coordinates "<< comm.coordinates()[0] <<", "<< comm.coordinates()[1] <<", "<< comm.coordinates()[2] <<'\n';

	auto comm_sub = comm.sub();
	static_assert( comm_sub.dimensionality == 2 , "!" );
	assert( comm_sub.num_elements() == 4 );
	assert( comm_sub.shape()[0] == 2 );
	assert( comm_sub.shape()[1] == 2 );
}
	return 0;
}

#endif
#endif

