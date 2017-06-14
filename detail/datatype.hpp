#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_DETAIL_DATATYPE -lboost_mpi -lboost_serialization -lboost_timer $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_DETAIL_DATATYPE_HPP
#define BOOST_MPI3_DETAIL_DATATYPE_HPP

#include<mpi.h>
#include<type_traits>
#include<utility> // pair

namespace boost{
namespace mpi3{
namespace detail{

template<class T> struct datatype;

template<> struct datatype<char          > : std::integral_constant<int, MPI_CHAR         >{};
template<> struct datatype<unsigned char > : std::integral_constant<int, MPI_UNSIGNED_CHAR>{};
template<> struct datatype<int           > : std::integral_constant<int, MPI_INT          >{};
template<> struct datatype<long          > : std::integral_constant<int, MPI_LONG         >{};
template<> struct datatype<float         > : std::integral_constant<int, MPI_FLOAT        >{};
template<> struct datatype<double        > : std::integral_constant<int, MPI_DOUBLE       >{};
template<> struct datatype<unsigned int  > : std::integral_constant<int, MPI_UNSIGNED     >{};
template<> struct datatype<unsigned long > : std::integral_constant<int, MPI_UNSIGNED_LONG>{};
template<> struct datatype<long double   > : std::integral_constant<int, MPI_LONG_DOUBLE>{};
template<> struct datatype<long long int>  : std::integral_constant<int, MPI_LONG_LONG_INT>{};
template<> struct datatype<std::pair<float, int>>: std::integral_constant<int, MPI_FLOAT_INT>{};
template<> struct datatype<std::pair<long, int>> : std::integral_constant<int, MPI_LONG_INT>{};
template<> struct datatype<std::pair<double, int>> : std::integral_constant<int, MPI_DOUBLE_INT>{};
template<> struct datatype<std::pair<short, int>> : std::integral_constant<int, MPI_SHORT_INT>{};
template<> struct datatype<std::pair<int, int>> : std::integral_constant<int, MPI_2INT>{};
template<> struct datatype<std::pair<long double, int>> : std::integral_constant<int, MPI_LONG_DOUBLE_INT>{};


template<class T, int = datatype<T>::value>
std::true_type is_builtin_type_aux(T const&);
std::false_type is_builtin_type_aux(...);

template<class T> struct is_builtin_type : decltype(is_builtin_type_aux(T{})){};

}}}

#if _TEST_BOOST_MPI3_DETAIL_DATATYPE

#include "alf/boost/mpi3/main.hpp"
#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;

int boost::mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){
	assert(( mpi3::detail::is_built_in_type<double>{} ));
	assert(( mpi3::detail::is_built_in_type<std::pair<double, int>>{} ));
	assert(( not mpi3::detail::is_built_in_type<std::pair<double, double>>{} ));
}

#endif
#endif

