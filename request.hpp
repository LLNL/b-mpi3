// © Alfredo A. Correa 2019-2021
#ifndef BOOST_MPI3_REQUEST_HPP
#define BOOST_MPI3_REQUEST_HPP

#include "../mpi3/detail/call.hpp"
#include "../mpi3/detail/iterator.hpp" // detail::data

#include "../mpi3/status.hpp"

// #define OMPI_SKIP_MPICXX 1  // https://github.com/open-mpi/ompi/issues/5157
#include<mpi.h>

#include<stdexcept>
#include<vector>

namespace boost {
namespace mpi3 {

struct [[nodiscard]] request {
	// in mpich MPI_Request is same as int
	MPI_Request impl_ = MPI_REQUEST_NULL;  // NOLINT(misc-non-private-member-variables-in-classes) TODO(correaa)

	auto handle() {return impl_;}

	request() = default;
	request(request const& other) = delete;
	request(request&& other) noexcept : impl_{std::exchange(other.impl_, MPI_REQUEST_NULL)}{}

	request& operator=(request const&) = delete;
	request& operator=(request&& other) noexcept {
		request(std::move(other)).swap(*this);
		return *this;
	}
	bool completed() const {
		int ret;  // NOLINT(cppcoreguidelines-init-variables) delayed init
		MPI_(Request_get_status)(impl_, &ret, MPI_STATUS_IGNORE);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast) for macro
		return ret != 0;
	}
	status get_status() const{
		int ignore = -1;
		return MPI_(Request_get_status)(impl_, &ignore);
	}
	void swap(request& other){std::swap(impl_, other.impl_);}
	void cancel(){MPI_Cancel(&impl_);}
	bool valid() const{return impl_ != MPI_REQUEST_NULL;}
	~request() noexcept {  // TODO(correaa) check it can be no noexcept and cancellable
		try {
			wait();
			if(impl_ != MPI_REQUEST_NULL) {MPI_Request_free(&impl_);}
		} catch(...) {
			std::terminate();
		}
	}
	void wait() {
		if(impl_ != MPI_REQUEST_NULL) {MPI_(Wait)(&impl_, MPI_STATUS_IGNORE);}  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast) for macro
	}
	status get() {
		status ret;  // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init) delayed initialization
		int s = MPI_Wait(&impl_, &ret.impl_);
		if(s != MPI_SUCCESS) {throw std::runtime_error("cannot wait on request");}
		return ret;
	}
	void start(){MPI_(Start)(&impl_);}
	status test() const{return get_status();}
};

inline std::vector<status> test_some(std::vector<request> const& requests) {
	int outcount = -1;
	std::vector<int> ignore(requests.size());
	std::vector<status> ret(requests.size());
	int s = MPI_Testsome(
		static_cast<int>(requests.size()),
		const_cast<MPI_Request*>(&(requests.data()->impl_)),  // NOLINT(cppcoreguidelines-pro-type-const-cast) TODO(correaa)
		&outcount,
		ignore.data(),
		&(ret.data()->impl_)
	);
	if(s != MPI_SUCCESS) {throw std::runtime_error{"cannot test some"};}
	return ret;
}

inline std::vector<int> completed_some(std::vector<request> const& requests) {
	int outcount = -1;
	std::vector<int> ret(requests.size());
	int s = MPI_Testsome(
		static_cast<int>(requests.size()),
		const_cast<MPI_Request*>(&(requests.data()->impl_)),  // NOLINT(cppcoreguidelines-pro-type-const-cast) TODO(correaa)
		&outcount,
		ret.data(),
		MPI_STATUSES_IGNORE  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast) for macro
	);
	if(s != MPI_SUCCESS) {throw std::runtime_error("cannot completed some");}
	ret.resize(outcount);
	return ret;
}

template<class ContRequestIterator, class Size>
void wait_all_n(ContRequestIterator it, Size n){
	MPI_Waitall(n, &detail::data(it)->impl_, MPI_STATUSES_IGNORE);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast) for macro
}

template<class ContRequestIterator>
void wait_all(ContRequestIterator it1, ContRequestIterator it2){
	wait_all_n(it1, std::distance(it1, it2));
}

template<class... Args>
void wait(Args&&... args){
	auto move_impl = [](request&& r)->MPI_Request{	MPI_Request ret = r.impl_;
		r.impl_ = MPI_REQUEST_NULL;
		return ret;
	};
	std::vector<MPI_Request> v{move_impl(std::move(args))...};
	MPI_Waitall(static_cast<int>(v.size()), v.data(), MPI_STATUSES_IGNORE);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast) for macro
}

template<class ContiguousIterator, class Size>
ContiguousIterator wait_any_n(ContiguousIterator it, Size n){
	int index = -1;
	int s = MPI_Waitany(n, &detail::data(it)->impl_, &index, MPI_STATUS_IGNORE);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast) for macro
	if(s != MPI_SUCCESS) {throw std::runtime_error("cannot wait any");}
	return it + index;
}

template<class ContiguousIterator>
ContiguousIterator wait_any(ContiguousIterator first, ContiguousIterator last){
	return wait_any_n(first, std::distance(first, last));
}

template<class ContiguousIterator, class Size>
std::vector<int> wait_some_n(ContiguousIterator it, Size n){
	int outcount = -1;
	std::vector<int> indices(n);
	int s = MPI_Waitsome(n, &detail::data(it)->impl_, &outcount, indices.data(), MPI_STATUSES_IGNORE);  // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
	if(s != MPI_SUCCESS) {throw std::runtime_error("cannot wait some");}
	indices.resize(outcount);
	return indices;
}

template<class ContiguousIterator>
std::vector<int> wait_some(ContiguousIterator first, ContiguousIterator last){
	return wait_some_n(first, std::distance(first, last));
}

namespace detail {

// this doesn't work in general because MPI_Request == int in mpich
//template<class FT, FT* F, class... Args, decltype(static_cast<enum error>((*F)(std::declval<Args>()..., std::declval<MPI_Request*>())))* = nullptr>
//BMPI3_NODISCARD("") mpi3::request call(Args... args) {
//	mpi3::request ret;  // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init) delayed initialization
//	auto const e = static_cast<enum error>((*F)(args..., &ret.impl_));  // NOLINT(clang-analyzer-optin.mpi.MPI-Checker) // non-blocking calls have wait in request destructor
//	if(e != mpi3::error::success) {throw std::system_error{e, "cannot call function " + std::string{__PRETTY_FUNCTION__}};}
//	return ret;
//}

template<class FT, FT* F, class... Args, decltype(static_cast<enum error>((*F)(std::declval<Args>()..., std::declval<MPI_Request*>())))* = nullptr>
mpi3::request call_i(Args... args) {
	mpi3::request ret;  // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init) delayed initialization
	auto const e = static_cast<enum error>((*F)(args..., &ret.impl_));  // NOLINT(clang-analyzer-optin.mpi.MPI-Checker) // non-blocking calls have wait in request destructor
	if(e != mpi3::error::success) {throw std::system_error{e, "cannot call function " + std::string{__PRETTY_FUNCTION__}};}  // NOLINT(clang-analyzer-optin.mpi.MPI-Checker) // MPI_Wait called on destructor of ret
	return ret;  // ret destructor will call wait
}  // NOLINT(clang-analyzer-optin.mpi.MPI-Checker)

#define MPI_I(F) detail::call_i<decltype(MPI_I##F), MPI_I##F>  // NOLINT(cppcoreguidelines-macro-usage): name concatenation

}  // end namespace detail

}  // end namespace mpi3
}  // end namespace boost

#endif
