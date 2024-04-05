// Copyright 2019-2024 Alfredo A. Correa

#include <fftw3-mpi.h>

#include <multi/adaptors/fftw.hpp>

#include <multi/array_ref.hpp>

#include <mpi3/allocator.hpp>
#include <mpi3/communicator.hpp>

namespace boost::mpi3::fftw {

struct environment {
	environment() { fftw_mpi_init(); }

	environment(environment const&) = delete;
	environment(environment&&)      = delete;

	environment& operator=(environment const&) = delete;
	environment& operator=(environment&&)      = delete;

	~environment() { fftw_mpi_cleanup(); }

	template<class... Args>
	static auto local_size_2d(Args... args) { return fftw_mpi_local_size_2d(args...); }

	template<class... Args>
	static auto local_size_many(Args... args) { return fftw_mpi_local_size_many(args...); }
};

template<class T>
using default_allocator =
	// std::allocator<T>
	boost::multi::fftw::allocator<T>
	// boost::mpi3::allocator<T>
	;

template<
	class T,
	boost::multi::dimensionality_type D,
	class Alloc = default_allocator<T>>
class array;

template<
	class T,
	boost::multi::dimensionality_type D,
	class Alloc = default_allocator<T>>
class unbalanced_array;

// namespace bmpi3 = boost::mpi3;

class local_2d_type {
    std::ptrdiff_t n0_     = -1;
    std::ptrdiff_t start0_ = -1;
    std::ptrdiff_t count_;

    public:
    local_2d_type(multi::extensions_t<2> const& ext, boost::mpi3::communicator& comm)
    : count_{environment::local_size_2d(std::get<0>(ext).size(), std::get<1>(ext).size(), &comm, &n0_, &start0_)} {}

    auto count() const { return count_; }
    auto extension() const { return multi::extension_t{start0_, start0_ + n0_}; }
};

template<class T, class Alloc>
class array<T, multi::dimensionality_type{2}, Alloc> {
	MPI_Comm handle_;
	Alloc    alloc_;

	typename std::allocator_traits<Alloc>::size_type                              local_count_;
	boost::multi::array_ptr<T, 2, typename std::allocator_traits<Alloc>::pointer> local_ptr_;
	// std::ptrdiff_t                                                                n0_;

    local_2d_type local_layout_;

	static auto
	local_2d(multi::extensions_t<2> ext, MPI_Comm handle) {  // boost::mpi3::communicator const& comm){
		std::ptrdiff_t local_n0      = 0;
		std::ptrdiff_t local_0_start = 0;

		auto count = fftw_mpi_local_size_2d(std::get<0>(ext).size(), std::get<1>(ext).size(), handle, &local_n0, &local_0_start);
		assert(count >= local_n0 * std::get<1>(ext).size());
		return std::pair<typename std::allocator_traits<Alloc>::size_type, multi::extensions_t<2>>(
			static_cast<typename std::allocator_traits<Alloc>::size_type>(count),
			multi::extensions_t<2>({local_0_start, local_0_start + local_n0}, std::get<1>(ext))
		);
	}

	static auto local_count_2d(multi::extensions_t<2> ext, MPI_Comm handle) {
		return local_2d(ext, handle).first;
	}
	static auto local_extension_2d(multi::extensions_t<2> ext, MPI_Comm handle) {
		return local_2d(ext, handle).second;
	}

 public:
	MPI_Comm handle() const { return handle_; }

	using element_type = T;

	array(multi::extensions_t<2> ext, boost::mpi3::communicator& comm, Alloc alloc = Alloc{})
	: handle_{&comm},
	  alloc_{alloc},
      local_layout_(ext, comm),
	  local_count_{local_count_2d(ext, &comm)},
	  local_ptr_{alloc_.allocate(local_count_), local_extension_2d(ext, &comm)}
    {}

	array(multi::extensions_t<2> ext, element_type const& e, boost::mpi3::communicator& comm, Alloc alloc = Alloc{})
	: handle_{&comm},
	  alloc_{alloc},
      local_layout_(ext, comm),
	  local_count_{local_count_2d(ext, handle_)},
	  local_ptr_{alloc_.allocate(local_count_), local_extension_2d(ext, handle_)}
	{
		std::uninitialized_fill_n(local_ptr_.base(), local_extension_2d(ext, handle_).num_elements(), e);
	}

	array(array const& other)
	: handle_{other.handle_},
	  alloc_{other.alloc_},
	  local_count_{other.local_count_},
	  local_ptr_{alloc_.allocate(local_count_), local_extension_2d(other.extensions(), handle_)}
    {
		this->local_cutout() = other.local_cutout();
	}

	// array(array&& other) noexcept
	// : handle_{other.handle_},
	//   alloc_{std::move(other.alloc_)},
    //   local_layout_(other.extensions(), other.communicator()),
	//   local_count_{std::exchange(other.local_count_, 0)},
	//   local_ptr_{std::exchange(other.local_ptr_, boost::multi::array_ptr<T, 2, typename std::allocator_traits<Alloc>::pointer>{nullptr})}
    // {}

	// explicit array(multi::array<T, 2> const& other, MPI_Comm handle, Alloc alloc = {})
	// : array(other.extensions(), handle, alloc) {
	// 	local_cutout() = other.stenciled(std::get<0>(local_cutout().extensions()), std::get<1>(local_cutout().extensions()));
	// }

	//  bool empty() const { return extensions().num_elements(); }

	boost::multi::array_ref<T, 2>  local_cutout() & { return *local_ptr_; }
	boost::multi::array_cref<T, 2> local_cutout() const& { return *local_ptr_; }

	ptrdiff_t local_count() const& { return local_count_; }

	// auto extensions() const& { return multi::extensions_t<2>{local_layout_.n0_, std::get<1>(local_cutout().extensions())}; }

	// ptrdiff_t num_elements() const& { return multi::layout_t<2>(extensions()).num_elements(); }

	template<class Array>
	static auto from_scatter(Array const& snd) -> array {
		array ret(snd.extensions());
		ret.scatter(snd);
		return ret;
	}

	// template<class Array>
	// void scatter(Array const& snd) & {
	// 	auto& comm = reinterpret_cast<boost::mpi3::communicator&>(handle_);

	// 	auto const sendcounts = comm |= static_cast<int>(local_cutout().num_elements());
	// 	auto const displs     = comm |= static_cast<int>(snd[local_cutout().extension().front()].base() - snd.base());

	// 	MPI_Scatterv(
	// 		snd.base(), sendcounts.data(), displs.data(), MPI_DOUBLE_COMPLEX,
	// 		local_cutout().base(), local_cutout().num_elements(), MPI_DOUBLE_COMPLEX,
	// 		0, &comm
	// 	);
	// }

	auto communicator() const -> boost::mpi3::communicator& {
		return const_cast<boost::mpi3::communicator&>(reinterpret_cast<boost::mpi3::communicator const&>(handle_));
	}

	// template<class Array>
	// void all_gather(Array&& rcv) const& {
	// 	assert(rcv.extensions() == extensions());

	// 	auto& comm = const_cast<boost::mpi3::communicator&>(reinterpret_cast<boost::mpi3::communicator const&>(handle_));

	// 	auto const recvcounts = comm |= static_cast<int>(local_cutout().num_elements());
	// 	auto const displs     = comm |= static_cast<int>(rcv[local_cutout().extension().front()].base() - rcv.base());

	// 	MPI_Allgatherv(
	// 		local_cutout().base(), local_cutout().num_elements(), MPI_DOUBLE_COMPLEX,
	// 		rcv.base(),
	// 		recvcounts.data(), displs.data(), MPI_DOUBLE_COMPLEX,
	// 		handle_
	// 	);
	// }

	// template<class Alloc2 = Alloc>
	// explicit operator multi::array<T, 2, Alloc2>() const& {
	// 	multi::array<T, 2, Alloc2> ret(extensions());
	// 	all_gather(ret);
	// 	return ret;
	// }

	// array& operator=(multi::array<T, 2> const& other) & {
	// 	if(other.extensions() == extensions())
	// 		local_cutout() = other.stenciled(std::get<0>(local_cutout().extensions()), std::get<1>(local_cutout().extensions()));
	// 	else {
	// 		array tmp{other};
	// 		std::swap(*this, tmp);
	// 	}
	// 	return *this;
	// }
	// bool operator==(multi::array<T, 2> const& other) const&{
	//  if(other.extensions() != extensions()) return false;
	//  return comm_&=(local_cutout() == other.stenciled(std::get<0>(local_cutout().extensions()), std::get<1>(local_cutout().extensions())));
	// }
	// friend bool operator==(multi::array<T, 2> const& other, array const& self){
	//  return self.operator==(other);
	// }
	// bool operator==(array<T, 2> const& other) const&{assert(comm_==other.comm_);
	//  return comm_&=(local_cutout() == other.local_cutout());
	// }
	// array& operator=(array const& other)&{
	//  if(other.extensions() == this->extensions() and other.comm_ == other.comm_)
	//      local_cutout() = other.local_cutout();
	//  else assert(0);
	//  return *this;
	// }
	~array() { alloc_.deallocate(local_cutout().data_elements(), local_count_); }
};

template<class Array>
auto scatter(Array const& arr) {
	return array<typename Array::element_type, Array::dimensionality>::from_scatter(arr);
}

#if 0
template<class T, class Alloc>
class array<T, multi::dimensionality_type{2}, Alloc> {
	boost::mpi3::communicator* commhandle_;

	Alloc alloc_;

	class local_2d_type {
		std::ptrdiff_t n0_     = -1;
		std::ptrdiff_t start0_ = -1;
		std::ptrdiff_t count_;

	 public:
		local_2d_type(multi::extensions_t<2> const& ext, boost::mpi3::communicator& comm)
		: count_{environment::local_size_2d(std::get<0>(ext).size(), std::get<1>(ext).size(), &comm, &n0_, &start0_)} {}

		auto count() const { return count_; }
		auto extension() const { return multi::extension_t{start0_, start0_ + n0_}; }
	} local_;

 public:
	using element_type = T;
	using element_ptr  = typename std::allocator_traits<Alloc>::pointer;

 private:
	boost::multi::array_ptr<element_type, 2, element_ptr> local_ptr_;

 public:
    auto local_count() const {return local_.count();}

	array(multi::extensions_t<2> ext, element_type const& e, boost::mpi3::communicator& comm, Alloc alloc = Alloc{})
	: commhandle_{&comm},
	  alloc_{alloc},
	  local_{ext, comm},
	  local_ptr_{alloc_.allocate(local_.count()), multi::extensions_t<2>(local_.extension(), std::get<1>(ext))} {
		std::uninitialized_fill(local_ptr_->elements().begin(), local_ptr_->elements().end(), e);  // TODO(correaa) use adl_uninit_fill or uninitialized_fill member
		// std::uninitialized_fill_n(local_ptr_->base(), local_ptr_->num_elements(), e);
	}

    array(array const&);

	boost::multi::array_ref<T, 2>  local() & { return *local_ptr_; }
	boost::multi::array_cref<T, 2> local() const& { return *local_ptr_; }

	boost::multi::array_cref<T, 2> clocal() const { return local(); }

	// template<class Array>
	// void scatter(Array const& snd) & {
	//     auto& comm = reinterpret_cast<boost::mpi3::communicator&>(handle_);

	//     auto const sendcounts = comm |= static_cast<int>(local_cutout().num_elements());
	//     auto const displs     = comm |= static_cast<int>(snd[local_cutout().extension().front()].base() - snd.base());

	//     MPI_Scatterv(
	//         snd.base(), sendcounts.data(), displs.data(), MPI_DOUBLE_COMPLEX,
	//         local_cutout().base(), local_cutout().num_elements(), MPI_DOUBLE_COMPLEX,
	//         0, &comm
	//     );
	// }

	// auto communicator() const -> boost::mpi3::communicator& {
	//     return const_cast<boost::mpi3::communicator&>(reinterpret_cast<boost::mpi3::communicator const&>(handle_));
	// }

	// template<class Array>
	// void all_gather(Array&& rcv) const& {
	//     assert(rcv.extensions() == extensions());

	//     auto& comm = const_cast<boost::mpi3::communicator&>(reinterpret_cast<boost::mpi3::communicator const&>(handle_));

	//     auto const recvcounts = comm |= static_cast<int>(local_cutout().num_elements());
	//     auto const displs     = comm |= static_cast<int>(rcv[local_cutout().extension().front()].base() - rcv.base());

	//     MPI_Allgatherv(
	//         local_cutout().base(), local_cutout().num_elements(), MPI_DOUBLE_COMPLEX,
	//         rcv.base(),
	//         recvcounts.data(), displs.data(), MPI_DOUBLE_COMPLEX,
	//         handle_
	//     );
	// }

	// template<class Alloc2 = Alloc>
	// explicit operator multi::array<T, 2, Alloc2>() const& {
	//     multi::array<T, 2, Alloc2> ret(extensions());
	//     all_gather(ret);
	//     return ret;
	// }

	// array& operator=(multi::array<T, 2> const& other) & {
	//     if(other.extensions() == extensions())
	//         local_cutout() = other.stenciled(std::get<0>(local_cutout().extensions()), std::get<1>(local_cutout().extensions()));
	//     else {
	//         array tmp{other};
	//         std::swap(*this, tmp);
	//     }
	//     return *this;
	// }
	// // bool operator==(multi::array<T, 2> const& other) const&{
	// //  if(other.extensions() != extensions()) return false;
	// //  return comm_&=(local_cutout() == other.stenciled(std::get<0>(local_cutout().extensions()), std::get<1>(local_cutout().extensions())));
	// // }
	// // friend bool operator==(multi::array<T, 2> const& other, array const& self){
	// //  return self.operator==(other);
	// // }
	// // bool operator==(array<T, 2> const& other) const&{assert(comm_==other.comm_);
	// //  return comm_&=(local_cutout() == other.local_cutout());
	// // }
	// // array& operator=(array const& other)&{
	// //  if(other.extensions() == this->extensions() and other.comm_ == other.comm_)
	// //      local_cutout() = other.local_cutout();
	// //  else assert(0);
	// //  return *this;
	// // }
	~array() { alloc_.deallocate(local().base(), local_.count()); }
};
#endif

template<class T, class Alloc>
class unbalanced_array<T, multi::dimensionality_type{2}, Alloc> {
	boost::mpi3::communicator* commhandle_;

	Alloc alloc_;

	class local_2d_type {
		std::ptrdiff_t n0_     = -1;
		std::ptrdiff_t start0_ = -1;
		std::ptrdiff_t count_;

	 public:
		local_2d_type(multi::extensions_t<2> const& ext, boost::mpi3::communicator& comm)
		: count_{environment::local_size_2d(std::get<0>(ext).size(), std::get<1>(ext).size(), &comm, &n0_, &start0_)} {}

		auto count() const { return count_; }
		auto extension() const { return multi::extension_t{start0_, start0_ + n0_}; }
	} local_;

 public:
	using element_type = T;
	using element_ptr  = typename std::allocator_traits<Alloc>::pointer;

 private:
	boost::multi::array_ptr<element_type, 2, element_ptr> local_ptr_;

 public:
	unbalanced_array(multi::extensions_t<2> ext, element_type const& e, boost::mpi3::communicator& comm, Alloc alloc = Alloc{})
	: commhandle_{&comm},
	  alloc_{alloc},
	  local_{ext, *commhandle_},
	  local_ptr_{alloc_.allocate(local_.count()), multi::extensions_t<2>(local_.extension(), std::get<1>(ext))} {
		std::uninitialized_fill(local_ptr_->elements().begin(), local_ptr_->elements().end(), e);  // TODO(correaa) use adl_uninit_fill or uninitialized_fill member
		// std::uninitialized_fill_n(local_ptr_->base(), local_ptr_->num_elements(), e);
	}

	boost::multi::array_ref<T, 2>  local() & { return *local_ptr_; }
	boost::multi::array_cref<T, 2> local() const& { return *local_ptr_; }

	boost::multi::array_cref<T, 2> clocal() const { return local(); }
};

}  // namespace boost::mpi3::fftw
