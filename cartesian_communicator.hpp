#if COMPILATION_INSTRUCTIONS
(echo '#include "'$0'"'>$0.cpp)&&mpic++ `#-Wfatal-errors` -D_TEST_BOOST_MPI3_CARTESIAN_COMMUNICATOR $0.cpp -o $0x&&mpirun -n 12 --oversubscribe $0x&&rm $0x $0.cpp;exit
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

template<dimensionality_type D = -1> struct cartesian_communicator;

template<>
struct cartesian_communicator<-1> : communicator{
	private:
	cartesian_communicator() : communicator(){}
	public:
	template<class Shape, class Period>
	cartesian_communicator(communicator& comm_old, Shape const& s, Period const& p){
		assert(s.size() == p.size());
		int status = MPI_Cart_create(&comm_old, s.size(), s.data(), p.data(), false, &impl_);
		if(status != MPI_SUCCESS) throw std::runtime_error("cannot create cart comm ");
		assert(impl_ != MPI_COMM_NULL);
		// there is an bug in mpich, in which the the remaining dim are none then the communicator is not well defined.
	}
	template<class Shape>
	cartesian_communicator(communicator& comm_old, Shape const& s) : cartesian_communicator(comm_old, s, std::vector<int>(s.size(), 0)){}
	
	cartesian_communicator(communicator& comm_old, std::initializer_list<int> shape) 
		: cartesian_communicator(comm_old, std::vector<int>(shape)){}
	cartesian_communicator(communicator& comm_old, std::initializer_list<int> shape, std::initializer_list<int> period) 
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
	template<class RemainDim>
	cartesian_communicator sub(RemainDim const& remain_dims) const{
		assert(remain_dims.size() == dimensionality());
		cartesian_communicator ret;
		MPI_(Cart_sub)(impl_, remain_dims.data(), &ret.impl_);
		return ret;
	}
	cartesian_communicator sub(std::initializer_list<int> il) const{
		return sub<std::vector<int>>(il);
	}
//	cartesian_communicator
	cartesian_communicator sub() const{
		std::vector<int> remain(1, dimensionality());
		return sub(remain);
	}
};

template<dimensionality_type D>
struct cartesian_communicator : cartesian_communicator<-1>{
	constexpr static dimensionality_type dimensionality = D;
	static auto dims_create(int nnodes, int ndims){
		std::vector<int> dims(ndims, 0);
		MPI_Dims_create(nnodes, ndims, dims.data());
		return dims;
	}
	explicit cartesian_communicator(communicator& other) : 
		cartesian_communicator<-1>(other, dims_create(other.size(), D))
	{}
};

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

	mpi3::cartesian_communicator<> comm(world, {4, 3}, {1, 0});
	cerr <<"I am rank "<< comm.rank() <<" and have coordinates "<< comm.coordinates()[0] <<", "<< comm.coordinates()[1] <<"\n";
}
	if(world.root()) cerr<<"---"<<std::endl;
{
	assert(world.size() == 12);

	mpi3::cartesian_communicator<3> comm{world};
	static_assert( mpi3::cartesian_communicator<3>::dimensionality == 3, "!");
	assert( comm.num_elements() == world.size() );
	assert( comm.shape()[0] == 3 );
	assert( comm.shape()[1] == 2 );
	assert( comm.shape()[2] == 2 );
	cerr<<"I am rank "<< comm.rank() <<" and have coordinates "<< comm.coordinates()[0] <<", "<< comm.coordinates()[1] <<", "<< comm.coordinates()[2] <<'\n';
}
	return 0;
}

#endif
#endif


