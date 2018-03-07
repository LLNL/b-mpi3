#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"">$0x.cpp) && mpic++ -O3 -std=c++14 -Wfatal-errors -D_BOOST_MPI3_MAIN_ENVIRONMENT -D_TEST_BOOST_MPI3_DETAIL_VALUE_TRAITS $0x.cpp -o $0x.x && time mpirun -n 1 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_DETAIL_VALUE_TRAITS_HPP
#define BOOST_MPI3_DETAIL_VALUE_TRAITS_HPP

#include "./datatype.hpp"
#include "./iterator.hpp"
//#include "./just.hpp"

#include<iterator>
#include<type_traits>

namespace boost{
namespace mpi3{
namespace detail{

template<class T> 
struct is_memcopyable : std::integral_constant<bool, std::is_trivially_copyable<T>{}>{};

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

struct value_unspecified{};
struct nonmemcopyable_serializable_tag{using base = value_unspecified;};
struct memcopyable_tag{using base = value_unspecified;};
struct basic_tag : memcopyable_tag{using base = memcopyable_tag;};

template<class V, typename = std::enable_if_t<is_memcopyable<V>{} and not is_basic<V>{}>>
memcopyable_tag value_category_aux(V&&);
template<class V, typename = std::enable_if_t<is_basic<V>{}>>
basic_tag value_category_aux(V&&);

template<class V>
struct value_category{using type = decltype(value_category_aux(V()));};

template<class V>
using value_category_t = typename value_category<V>::type;

}}}

#ifdef _TEST_BOOST_MPI3_DETAIL_VALUE_TRAITS

#include "../../mpi3/environment.hpp"
#include "../../mpi3/main.hpp"

#include<deque>
#include<list>
#include<vector>

#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;

template<class It>
std::string f(It it, mpi3::detail::memcopyable_tag){
	return "memcopyable_tag";
};

template<class It>
std::string f(It it, mpi3::detail::basic_tag const&){
	return "basic_tag";
};

template<class It> std::string f(It&& it){
	return f(
		std::forward<It>(it),
		typename boost::mpi3::detail::value_category<typename std::iterator_traits<It>::value_type>::type{}
	);
}

int mpi3::main(int, char*[], mpi3::environment&){
	{
		std::list<std::tuple<double, double>> l1;
		assert( f(l1.begin()) == "memcopyable_tag" );
		std::list<double> l2;
		assert( f(l2.begin()) == "basic_tag" );
	}
	return 0;
}
#endif
#endif

