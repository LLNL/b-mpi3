//  -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
//#if COMPILATION
//OMPI_CXX=$CXXX OMPI_CXXFLAGS=$CXXFLAGS mpic++  $0 -o $0x&&mpirun -n 6 --oversubscribe $0x;exit
//#endif
// Copyright 2018-2021 Alfredo A. Correa

#ifndef BOOST_MPI3_CARTESIAN_COMMUNICATOR_HPP
#define BOOST_MPI3_CARTESIAN_COMMUNICATOR_HPP

#include "../mpi3/communicator.hpp"
#include "../mpi3/process.hpp"

#include "../mpi3/detail/call.hpp"

namespace boost {
namespace mpi3 {

using dimensionality_type = int;
static constexpr dimensionality_type dynamic_extent = -1;

template<dimensionality_type D = dynamic_extent> struct cartesian_communicator;

template<>
struct cartesian_communicator<dynamic_extent> : communicator{

	cartesian_communicator() = default;

	cartesian_communicator(cartesian_communicator const&) = delete;
	cartesian_communicator(cartesian_communicator     &&) = default;
	// vvv---  this is an unusual "duplicate" constructor
	cartesian_communicator(cartesian_communicator& other) : communicator{other} {}  // NOLINT(hicpp-use-equals-default,modernize-use-equals-default) cannot be defaulted because bug in nvcc 11

	template<class Shape, class Period>
	cartesian_communicator(communicator& comm_old, Shape const& s, Period const& p){
		assert(s.size() == p.size());
		MPI_(Cart_create)(comm_old.get(), s.size(), s.data(), p.data(), false, &impl_);
	//	assert(impl_ != MPI_COMM_NULL); // null communicator is a valid outcome
		// TODO(correaa) try with mpich, WAS: there is an bug in mpich, in which if the remaining dim are none then the communicator is not well defined.
	}
	template<class Shape>
	cartesian_communicator(communicator& comm_old, Shape const& s) : cartesian_communicator(comm_old, s, std::vector<int>(s.size(), true)){}

	cartesian_communicator(communicator& comm_old, std::initializer_list<int> shape) 
		: cartesian_communicator(comm_old, std::vector<int>(shape)){}
	cartesian_communicator(communicator& comm_old, std::initializer_list<int> shape, std::initializer_list<int> period) 
		: cartesian_communicator(comm_old, std::vector<int>(shape), std::vector<int>(period)){}

	[[deprecated("use dimensionality() instead of dimension")]] 
	int dimension() const {int ret; MPI_Cartdim_get(impl_, &ret); return ret;}  // NOLINT(cppcoreguidelines-init-variables) delayed init

	cartesian_communicator& operator=(cartesian_communicator const&) = delete;
	cartesian_communicator& operator=(cartesian_communicator     &&) = default;
	// vvv nvcc 11 workaround, needs explicit definition of duplicate assigment
	cartesian_communicator& operator=(cartesian_communicator      & other) {  // NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator) "duplicate" assignment
		if(this == std::addressof(other)) {return *this;}  // lints cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator
		communicator::operator=(other);
		return *this;
	}

	~cartesian_communicator() = default;

	int dimensionality() const{int ret; MPI_(Cartdim_get)(impl_, &ret); return ret;}  // NOLINT(cppcoreguidelines-init-variables) delayed init

	std::vector<int> coordinates() const{
		std::vector<int> ret(dimensionality());
		MPI_(Cart_coords)(impl_, rank(), dimensionality(), ret.data());
		return ret;
	}

	auto topology() const{
		auto maxdims = dimensionality();
		class topology_t {
			std::vector<int> dimensions_;
			std::vector<int> periods_;
			std::vector<int> coordinates_;
			friend mpi3::cartesian_communicator<dynamic_extent>;
		 public:
			explicit topology_t(std::size_t n) : dimensions_(n), periods_(n), coordinates_(n) {}

			auto const& dimensions() const {return dimensions_;}
			auto const& periods() const {return periods_;}
			auto const& coordinates() const {return coordinates_;}
		} ret(maxdims);

		MPI_(Cart_get)(impl_, maxdims, ret.dimensions_.data(), ret.periods_.data(), ret.coordinates_.data());

		assert( ret.coordinates() == coordinates() );
		return ret;
	}

	std::vector<int>  shape()   const {return topology().dimensions();}
	std::vector<bool> periods() const {auto ps = topology().periods(); return {ps.begin(), ps.end()};}
	auto num_elements() const {return size();}

	template<class Coord>
	auto operator()(Coord const& coord){
		int rank = -1;
		MPI_(Cart_rank)(impl_, coord.data(), &rank);
		return (*this)[rank];
	//	return operator[](rank);
	}
	// int MPI_Cart_map not implemented
	cartesian_communicator sub_aux(std::vector<int> const& remain_dims) {
		assert( static_cast<dimensionality_type>(remain_dims.size()) == dimensionality() );
		cartesian_communicator ret; 
		MPI_(Cart_sub)(impl_, remain_dims.data(), &ret.impl_); 
		return ret;
	}

	template<class RemainDim = std::initializer_list<bool>>
	cartesian_communicator sub(RemainDim const& remain_dims) {
		return sub_aux(std::vector<int>(remain_dims.begin(), remain_dims.end()));
	}
	cartesian_communicator sub() {
		assert( dimensionality()>1 );
		std::vector<int> remain(dimensionality(), 1 /*true*/); remain[0] = 0/*false*/;
		return sub_aux(remain);
	}
};

enum fill_t{fill = 0};

template<dimensionality_type D>
struct cartesian_communicator : cartesian_communicator<>{

	cartesian_communicator() = default;

	cartesian_communicator(cartesian_communicator& other) : cartesian_communicator<>{other}{}
	cartesian_communicator(cartesian_communicator const&) = delete;
	cartesian_communicator(cartesian_communicator&&) noexcept = default;

	~cartesian_communicator() = default;

	static std::array<int, D> division(int nnodes, std::array<int, D> suggest = {}){
		return MPI_(Dims_create)(nnodes, D, suggest.data()), suggest;
	}
	constexpr static dimensionality_type dimensionality = D;
	cartesian_communicator(communicator& other, std::array<int, D> dims)
	try : cartesian_communicator<>(other, division(other.size(), dims)) {}
	catch(std::runtime_error& e) {
		std::ostringstream ss;
		std::copy(dims.begin(), dims.end(), std::ostream_iterator<int>{ss, " "});
		throw std::runtime_error{"cannot create cartesian communicator with constrains "+ss.str()+" from communicator of size "+std::to_string(other.size())+" because "+e.what()};
	}
	auto topology() const {
		struct topology_t{
			std::array<int, dimensionality> dimensions, periods, coordinates;
		} ret;
		MPI_(Cart_get)(
			impl_, dimensionality, 
			ret.dimensions.data(), ret.periods.data(), ret.coordinates.data()
		);
		return ret;
	}
	auto dimensions() const {return topology().dimensions;}

	cartesian_communicator& operator=(cartesian_communicator const&) = delete;
	cartesian_communicator& operator=(cartesian_communicator     &&) noexcept = default;
	// vvv  nvcc 11 workaround, needs explicit definition of duplicate assigment
	cartesian_communicator& operator=(cartesian_communicator      & other) {  // NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator) duplicate assignment
		if(this == std::addressof(other)) {return *this;}  // lints cert-oop54-cpp
		cartesian_communicator<>::operator=(other);
		return *this;
	}

	cartesian_communicator<D-1> sub() {
		static_assert( D != 1 , "!");
		auto comm_sub = cartesian_communicator<>::sub();
		return static_cast<cartesian_communicator<D-1>&>(comm_sub);
	}
	cartesian_communicator sub(std::array<int, D> const& remain_dims) {
		cartesian_communicator ret; MPI_Cart_sub(impl_, remain_dims.data(), &ret.get()); return ret;
	}
	cartesian_communicator<1> axis(int d) const {
		cartesian_communicator<1> ret;
		std::array<int, D> remains = {}; remains[d] = true;
		MPI_(Cart_sub)(impl_, remains.data(), &ret.get());
		return ret;

	}

	cartesian_communicator<D - 1> hyperplane(int d) const{
		static_assert( D != 1 , "hyperplane not possible for 1D communicators");
		
		cartesian_communicator<D - 1> ret;
		std::array<int, D> remains;
		for(auto & rem : remains) {rem = true;}
		remains[d] = false;
		MPI_(Cart_sub)(impl_, remains.data(), &ret.get());
		int dim = -1;
		MPI_Cartdim_get(ret.get(), &dim);
		assert(dim == D - 1);
		return ret;
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

#if not __INCLUDE_LEVEL__ // def _TEST_BOOST_MPI3_CARTESIAN_COMMUNICATOR

#include<iostream>

#include "../mpi3/main.hpp"
#include "../mpi3/version.hpp"
#include "../mpi3/ostream.hpp"

using std::cout;
using std::cerr;
namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], boost::mpi3::communicator world){
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
	assert(world.size() == 6);
	auto div = mpi3::cartesian_communicator<2>::division(6, {2});
	assert( div[0]*div[1] == 6 );
	assert( div[0] == 2 );
	assert( div[1] == 3 );
}
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
{
	assert(world.size() == 6);
	mpi3::cartesian_communicator<2> cart_comm(world, {2, 3});
	assert( cart_comm );
	assert( cart_comm.dimensions()[0] == 2 );
	assert( cart_comm.dimensions()[1] == 3 );
	auto row = cart_comm.axis(0);
	auto col = cart_comm.axis(1);
	assert( row.size() == 2 );
	assert( col.size() == 3 );
}
{
	assert(world.size() == 6);
	if(mpi3::cartesian_communicator<2> cart_comm{world, {2, 2}}){
		auto row = cart_comm.axis(0);
		auto col = cart_comm.axis(1);
	}
}
try{
	assert(world.size() == 6);
	mpi3::cartesian_communicator<2> cart_comm(world, {4});
	assert(cart_comm.dimensions()[0] == 2);
	assert(cart_comm.dimensions()[1] == 3);
}catch(...){}
{
	mpi3::cartesian_communicator<2> cart_comm(world, {2, mpi3::fill});
	assert(cart_comm.dimensions()[0] == 2);
	assert(cart_comm.dimensions()[1] == 3);
}
{
	mpi3::cartesian_communicator<2> cart_comm(world, {mpi3::fill, 2});
	assert(cart_comm.dimensions()[0] == 3);
	assert(cart_comm.dimensions()[1] == 2);
}
{
	return 0;

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
#ifdef __cpp_deduction_guides
{
	assert(world.size() == 12);

	mpi3::cartesian_communicator comm(world, {4, 3}, {true, false});
	assert( comm.dimensionality() == 2 );
	cerr <<"I am rank "<< comm.rank() <<" and have coordinates "<< comm.coordinates()[0] <<", "<< comm.coordinates()[1] <<"\n";
}
#endif
{
	assert(world.size() == 12);

	mpi3::cartesian_communicator<3> comm(world, {});
	static_assert( mpi3::cartesian_communicator<3>::dimensionality == 3, "!");
	assert( comm.cartesian_communicator<>::dimensionality() == 3 );
	assert( comm.num_elements() == world.size() );
	assert( comm.shape()[0] == 3 );
	assert( comm.shape()[1] == 2 );
	assert( comm.shape()[2] == 2 );
	cerr<<"+ I am rank "<< comm.rank() <<" and have coordinates "<< comm.coordinates()[0] <<", "<< comm.coordinates()[1] <<", "<< comm.coordinates()[2] <<'\n';

	auto comm_sub = comm.sub();
	static_assert( comm_sub.dimensionality == 2 , "!" );
	std::cout << "numelements " << comm_sub.num_elements() << std::endl;
	assert( comm_sub.num_elements() == 4 );

	assert( comm_sub.shape()[0] == 2 );
	assert( comm_sub.shape()[1] == 2 );
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

	{
		auto plane0 = comm.hyperplane(0);
		static_assert( plane0.dimensionality == 2 , "!" );
		assert( plane0.num_elements() == 4 );
		assert( plane0.shape()[0] == 2 );
		assert( plane0.shape()[1] == 2 );
	}

	{
		auto plane1 = comm.hyperplane(1);
		static_assert( plane1.dimensionality == 2 , "!" );
		assert( plane1.num_elements() == 6 );
		assert( plane1.shape()[0] == 3 );
		assert( plane1.shape()[1] == 2 );
	}
	
	{
		auto plane2 = comm.hyperplane(2);
		static_assert( plane2.dimensionality == 2 , "!" );
		assert( plane2.num_elements() == 6 );
		assert( plane2.shape()[0] == 3 );
		assert( plane2.shape()[1] == 2 );
	}
	
}
	return 0;
}

#endif
#endif
