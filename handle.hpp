/* -*- indent-tabs-mode: t -*- */
#ifndef BOOST_MPI3_DETAIL_HANDLE_HPP
#define BOOST_MPI3_DETAIL_HANDLE_HPP

//#include<boost/exception/to_string.hpp>

#include<cassert>
#include<stdexcept> // runtime_error
#include<string>

#include<mpi.h>

namespace boost{
namespace mpi3{
namespace detail{

template<class Self, class Impl>
struct caller{
	Impl& impl(){return static_cast<Self&>(*this).impl_;}
	Impl const& impl() const{return static_cast<Self const&>(*this).impl_;}

	template<class F, class... Args>
	void static_call(F f, Args&&... args){
		int status = f(std::forward<Args>(args)...);
		if(status != 0) {throw std::runtime_error{"error "+ std::to_string(status)};}
	}
	template<int(*F)(Impl, char const*, char const*)> void call(
		char const* c1, char const* c2
	){
		int status = F(impl(), c1, c2);
		if(status != 0) {throw std::runtime_error{"error "+ std::to_string(status)};}
	}
	template<int(*F)(Impl, char const*, char const*)> void call(
		std::string const& s1, std::string const& s2
	){
		return call<F>(s1.c_str(), s2.c_str());
	}
	template<int(*F)(Impl, int*)> int call() const{
		int ret = -1;
	//	static_call(F, impl_, &ret);
		int status = F(impl(), &ret);
		if(status != MPI_SUCCESS) {throw std::runtime_error{"error " + std::to_string(status)};}
		return ret;
	}
	template<int(*F)(Impl, int, char*)> std::string call(int n) const{
		std::array<char, MPI_MAX_INFO_KEY> ret{};
		int status = F(impl(), n, ret.data());
		if(status != 0) {throw std::runtime_error{"error "+ std::to_string(status)};}
		return std::string{ret.data()};
	}
	template<int(*F)(Impl, char const*, int*, int*)> std::pair<int, int> call(std::string const& key) const{
		int flag;  // NOLINT(cppcoreguidelines-init-variables) delayed init
		int valuelen;  // NOLINT(cppcoreguidelines-init-variables) delayed init
		int status = F(impl(), key.c_str(), &valuelen, &flag);
		if(status != 0) {throw std::runtime_error{"error "+ std::to_string(status)};}
		return {valuelen, flag};
	}
	template<int(*F)(Impl, char const*, int, char*, int*)> std::pair<std::string, int> call(std::string const& key, int valuelen) const{
		std::array<char,  MPI_MAX_INFO_VAL> value{};
		int flag;  // NOLINT(cppcoreguidelines-init-variables) delayed init
		int status = F(impl(), key.c_str(), valuelen, value.data(), &flag);
		if(status != 0) {throw std::runtime_error{"error "+ std::to_string(status)};}
		return {std::string(value.data(), valuelen), flag};
	}
	template<int(*F)(Impl, char const*)> void call(std::string const& key) const{
		int status = F(impl(), key.c_str());
		if(status != 0) {throw std::runtime_error("error "+ std::to_string(status));}
	}
};

template<
	class Self,
	class Impl,
	int(*CreateFunction)(Impl*),
	int(*DupFunction)(Impl, Impl*),
	int(*FreeFunction)(Impl*)
>
// TODO(correaa) rename as `indirect`
struct regular_handle : caller<regular_handle<Self, Impl, CreateFunction, DupFunction, FreeFunction>, Impl> {
	using caller<regular_handle<Self, Impl, CreateFunction, DupFunction, FreeFunction>, Impl>::call;
	using impl_t = Impl;
	impl_t impl_;  // NOLINT(misc-non-private-member-variables-in-classes) TODO(correaa)

	regular_handle() {CreateFunction(&impl_);}
	regular_handle(regular_handle&&) = delete;  // TODO(correaa) : introspect if "null state" is valid
	regular_handle(regular_handle const& other) {  // TODO(correaa) : revise in what cases a regular const& is correct
		int status = DupFunction(other.impl_, &impl_);
		if(status != MPI_SUCCESS) {throw std::runtime_error{"cannot copy handle"};}
	}
	~regular_handle() {
		assert(impl_ != MPI_INFO_NULL);
		if(impl_ != MPI_INFO_NULL) {FreeFunction(&impl_);}
	}
	void swap(Self& other) {std::swap(impl_, other.impl_);}
	regular_handle& operator=(regular_handle const& other) {
		if(this == &other) {return *this;}
		regular_handle tmp{other};
		swap(tmp);
		return *this;
	}
	regular_handle operator=(regular_handle&&) = delete;
};

//template<class Self, class Impl, int(*CreateFunction)(Impl*), int(*FreeFunction)(Impl*)>
//struct noncopyable_handle : caller<noncopyable_handle<Self, Impl, CreateFunction, FreeFunction>, Impl>{
//	using impl_t = Impl;
//	impl_t impl_;  // NOLINT(misc-non-private-member-variables-in-classes) TODO(correaa)
//	bool predefined_ = false;

//	noncopyable_handle(Impl code) : impl_(code), predefined_{true} {}
//	noncopyable_handle() {CreateFunction(&impl_);}
//	noncopyable_handle(noncopyable_handle const&) = delete;
//	~noncopyable_handle() {
//		assert(impl_ != MPI_INFO_NULL);
//	//	if(impl_ != MPI_INFO_NULL) 
//		if(not predefined_) {FreeFunction(&impl_);}
//	}
//	void swap(noncopyable_handle& other){std::swap(impl_, other.impl_);}
//	Self& operator=(noncopyable_handle const& other) = delete;
//};

struct uninitialized{};

template<class Self, class Impl, int(*FreeFunction)(Impl*)>
struct nondefault_handle : caller<nondefault_handle<Self, Impl, FreeFunction>, Impl>{
 private:
	using impl_t = Impl;

 public:
	impl_t impl_;  // NOLINT(misc-non-private-member-variables-in-classes) TODO(correaa)
	bool predefined_ = false;  // NOLINT(misc-non-private-member-variables-in-classes) TODO(correaa)

	explicit nondefault_handle(Impl code) : impl_(code), predefined_(true){}
	nondefault_handle() = delete;
	explicit nondefault_handle(uninitialized /*unused*/){};
	nondefault_handle(nondefault_handle const&) = delete;
	nondefault_handle(nondefault_handle&&) = delete;
	~nondefault_handle(){if(not predefined_) {FreeFunction(&impl_);}}
	void swap(nondefault_handle& other){std::swap(impl_, other.impl_);}
	nondefault_handle& operator=(nondefault_handle const& other) = delete;
	nondefault_handle& operator=(nondefault_handle&& other) = delete;
};

//template<class Self, class Impl>
//struct persistent_handle : caller<persistent_handle<Self, Impl>, Impl>{
//private:
//	using impl_t = Impl;
//	impl_t impl_;

//public:
//	persistent_handle() = delete;
//	persistent_handle(uninitialized){};
//	persistent_handle(Impl code) : impl_(code){}
//	persistent_handle(persistent_handle const&) = delete;
//	~persistent_handle() = default;
//};

}  // end namespace detail
}  // end namespace mpi3
}  // end namespace boost

#endif

