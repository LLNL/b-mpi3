//  -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
// Copyright 2018-2022 Alfredo A. Correa

#ifndef BOOST_MPI3_CARTESIAN_COMMUNICATOR_HPP
#define BOOST_MPI3_CARTESIAN_COMMUNICATOR_HPP

#include "../mpi3/communicator.hpp"
#include "../mpi3/process.hpp"

#include "../mpi3/detail/call.hpp"

namespace boost::mpi3 {

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

	int dimensionality() const {int ret; MPI_(Cartdim_get)(impl_, &ret); return ret;}  // NOLINT(cppcoreguidelines-init-variables) delayed init

	std::vector<int> coordinates() const {
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

enum fill_t{fill = 0, _ = 0};

template<dimensionality_type D>
struct cartesian_communicator : cartesian_communicator<> {
	cartesian_communicator() = default;

	cartesian_communicator(cartesian_communicator& other) : cartesian_communicator<>{other}{}
	cartesian_communicator(cartesian_communicator const&) = delete;
	cartesian_communicator(cartesian_communicator&&) noexcept = default;

	~cartesian_communicator() = default;

	static std::array<int, D> division(int nnodes, std::array<int, D> suggest = {}) {
		MPI_(Dims_create)(nnodes, D, suggest.data());
		return suggest;
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
		struct topology_t {
			std::array<int, dimensionality> dimensions, periods, coordinates;
		} ret = {};
		MPI_(Cart_get)(
			impl_, dimensionality,
			ret.dimensions.data(), ret.periods.data(), ret.coordinates.data()
		);
		return ret;
	}

	static constexpr dimensionality_type dimensions() {return D;}

	cartesian_communicator& operator=(cartesian_communicator const&) = delete;
	cartesian_communicator& operator=(cartesian_communicator     &&) noexcept = default;
	// vvv  nvcc 11 workaround, needs explicit definition of duplicate assigment
	cartesian_communicator& operator=(cartesian_communicator      & other) {  // NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator) duplicate assignment
		if(this == std::addressof(other)) {return *this;}  // lints cert-oop54-cpp
		cartesian_communicator<>::operator=(other);
		return *this;
	}

	cartesian_communicator<1> axis(int d) {
		cartesian_communicator<1> ret;
		std::array<int, D> remains{}; remains.fill(false);
		remains[d] = true;  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
		MPI_(Cart_sub)(impl_, remains.data(), &ret.get());
		return ret;

	}

	cartesian_communicator<D - 1> hyperplane(int d) {
		static_assert( D != 1 , "hyperplane not possible for 1D communicators");

		cartesian_communicator<D - 1> ret;
		std::array<int, D> remains{}; remains.fill(true);
		remains[d] = false;  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index)
		MPI_(Cart_sub)(impl_, remains.data(), &ret.get());
		assert(ret.cartesian_communicator<>::dimensionality() == D - 1);
		return ret;
	}
};

}  // end namespace boost::mpi3
#endif
