#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && c++ -O3 -std=c++11 -Wfatal-errors -D_TEST_BOOST_MPI3_DETAIL_IS_MEMCOPYABLE -lboost_serialization $0x.cpp -o $0x.x && $0x.x $@ && rm -f $0x.cpp $0x.x; exit
#endif
#ifndef BOOST_MPI3_DETAIL_IS_MEMCOPYABLE_HPP
#define BOOST_MPI3_DETAIL_IS_MEMCOPYABLE_HPP

#include<type_traits>
#include<utility> // pair
#include<tuple>
ewewqeqw
namespace boost{
namespace mpi3{

namespace detail{

template<class T> 
struct is_memcopyable : std::is_trivially_copyable<T>{};

template<class T, class... Ts> 
struct is_memcopyable<std::tuple<T, Ts...>> : 
	std::integral_constant<bool, 
		is_memcopyable<T>{} and is_memcopyable<std::tuple<Ts...>>{}
	>
{};

template<class T1, class T2> 
struct is_memcopyable<std::pair<T1, T2>> : 
	std::integral_constant<bool, 
		is_memcopyable<T1>{} and is_memcopyable<T2>{}
	>
{};

}}}

#if _TEST_BOOST_MPI3_DETAIL_IS_MEMCOPYABLE

#include<iostream>
#include<string>
#include<boost/type_index.hpp>
#include<cassert>

namespace mpi3 = boost::mpi3;
using std::cout;

struct A{
	A(A const&) = default;
	double d;
	std::tuple<int, double> t;
	A& operator=(A const&) = default;
	~A() = default;
	private:
	int i;
};

namespace boost{namespace mpi3{namespace detail{
template<> struct is_memcopyable<A> : std::true_type{};
}}}

int main(int argc, char* argv[]){

	using std::tuple;
	using std::pair;

	tuple<double, tuple<char, int>> t1 = {5.1, {'c', 8}};
	tuple<double, tuple<char, int>> t2;
	t2 = t1;
	tuple<double, tuple<char, int>> t3;
	std::memcpy(&t3, &t1, sizeof(t1));
	assert(t3 == t2);

	static_assert( mpi3::detail::is_memcopyable<A>{}, "" );
	static_assert( mpi3::detail::is_memcopyable<double>{}, "" );
	static_assert( mpi3::detail::is_memcopyable<tuple<double, A>>{}, "" );
	static_assert( mpi3::detail::is_memcopyable<pair<double, int>>{}, "" );
	cout << sizeof(double) << '\n';
	cout << sizeof(int) << '\n';
	cout << sizeof(pair<double, int>) << '\n';
//	assert(sizeof(double) + sizeof(int) == sizeof(std::pair<double, int>));
}

#endif
#endif

