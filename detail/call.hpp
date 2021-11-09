#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && mpic++ -O3 -std=c++17 `#-Wfatal-errors` -D_TEST_MPI3_DETAIL_CALL $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
//  (C) Copyright Alfredo A. Correa 2019

#ifndef BOOST_MPI3_DETAIL_CALL_HPP
#define BOOST_MPI3_DETAIL_CALL_HPP

#include "../error.hpp"
#include "../status.hpp"

// #define OMPI_SKIP_MPICXX 1  // https://github.com/open-mpi/ompi/issues/5157
#include<mpi.h> // MPI_MAX_PROCESSOR_NAME

#include<string>

namespace boost {
namespace mpi3 {
namespace detail {

template< class ...Args> struct back;
template< class A> struct back<A> { using type = A; };
template< class A, class... Args> struct back<A,Args...>{ 
	using type = typename back<Args...>::type;
};

template<class R, class... Args>
typename back<Args...>::type back_arg_aux(R(*pp)(Args...));

template<class F>
struct back_arg{
	using type = decltype(back_arg_aux(std::declval<F*>()));
};

template<class F> using back_arg_t = typename back_arg<F>::type;

template<int(*F)(char*, int*)>
std::string call() {
	int len = -1;
	std::array<char, MPI_MAX_PROCESSOR_NAME> name{};
	F(name.data(), &len);
	return {name.data(), static_cast<std::size_t>(len)};
}

template<class FT, FT* F, class... Args, decltype(static_cast<enum error>((*F)(std::declval<Args>()...)))* = nullptr>
void call(Args... args) {
	auto const e = static_cast<enum error>((*F)(args...));  // NOLINT(clang-analyzer-optin.mpi.MPI-Checker) // non-blocking calls have wait in request destructor
	if(e != mpi3::error::success) {throw std::system_error{e, "cannot call function " + std::string{__PRETTY_FUNCTION__}};}
}

template<class FT, FT* F, class... Args, decltype(static_cast<enum error>((*F)(std::declval<Args>()..., std::declval<MPI_Status*>())))* = nullptr>
[[nodiscard]] status call(Args... args) {
	mpi3::status ret;  // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init) delayed initialization
	auto const e = static_cast<enum error>((*F)(args..., &ret.impl_));  // NOLINT(clang-analyzer-optin.mpi.MPI-Checker) // non-blocking calls have wait in request destructor
	if(e != mpi3::error::success) {throw std::system_error{e, "cannot call function " + std::string{__PRETTY_FUNCTION__}};}
	return ret;
}

#if __cpp_nontype_template_parameter_auto >= 201606
template<auto F, class... Args, decltype(static_cast<enum error>(F(std::declval<Args>()...)))* =0>
void call(Args... args) {
	auto const e = static_cast<enum error>(F(args...));
	if(e != mpi3::error::success) {throw std::system_error{e, "cannot call function"};}
}

//template<auto F, class... Args>
//status call(Args... args) {
//	status ret;
//	auto e = static_cast<enum error>(F(args..., &ret.impl));
//	if(e != mpi3::error::success) {throw std::system_error{e, "cannot call function"};}
//}
#endif

#define MPI3_CALL(F) detail::call<decltype(F), F>  // NOLINT(cppcoreguidelines-macro-usage)
#define MPI_(F) MPI3_CALL(MPI_##F)  // NOLINT(cppcoreguidelines-macro-usage): name concatenation

}  // end namespace detail
}  // end namespace mpi3
}  // end namespace boost

#ifdef _TEST_MPI3_DETAIL_CALL

void f(double, int){}

int main(){
//	static_assert( 
//	typename boost::mpi3::detail::last_argument<decltype(f)>::type a;
}
#endif

#endif

