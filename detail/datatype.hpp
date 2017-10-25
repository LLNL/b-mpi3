#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpic++ -O3 -std=c++11 -Wfatal-errors -D_TEST_BOOST_MPI3_DETAIL_DATATYPE -lboost_serialization $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp $0x.x; exit
#endif
#ifndef BOOST_MPI3_DETAIL_DATATYPE_HPP
#define BOOST_MPI3_DETAIL_DATATYPE_HPP

#include<mpi.h>

#include<type_traits>
#include<utility> // pair
#include<complex>

namespace boost{
namespace mpi3{

namespace detail{

using float_int = std::pair<float, int>;
using long_int = std::pair<long, int>;
using double_int = std::pair<double, int>;
using short_int = std::pair<short, int>;
using int_int = std::pair<int, int>;
using long_double_int = std::pair<long double, int>;

using float_float = std::pair<float, float>;
using double_double = std::pair<double, double>;
using long_double_long_double = std::pair<long double, long double>;


using cxx_float_complex = std::complex<float>;
using cxx_double_complex = std::complex<double>;
using cxx_long_double_complex = std::complex<long double>;

using cxx_bool = bool;

template<class T> struct basic_datatype;

#define BOOST_MPI3_DECLARE_DATATYPE(TypE, MpiiD) \
template<> struct basic_datatype<TypE>{ \
	constexpr operator MPI_Datatype() const{return value;} \
	static constexpr MPI_Datatype value = MpiiD; \
}

// basic data types http://beige.ucs.indiana.edu/I590/node100.html

BOOST_MPI3_DECLARE_DATATYPE(char, MPI_CHAR);
BOOST_MPI3_DECLARE_DATATYPE(unsigned char, MPI_UNSIGNED_CHAR);

//BOOST_MPI3_DECLARE_DATATYPE(__int8, MPI_BYTE);
//BOOST_MPI3_DECLARE_DATATYPE(wchar, MPI_WCHAR_T);

BOOST_MPI3_DECLARE_DATATYPE(short, MPI_SHORT);
BOOST_MPI3_DECLARE_DATATYPE(unsigned short, MPI_UNSIGNED_SHORT);
BOOST_MPI3_DECLARE_DATATYPE(int, MPI_INT);
BOOST_MPI3_DECLARE_DATATYPE(unsigned int, MPI_UNSIGNED);
BOOST_MPI3_DECLARE_DATATYPE(long, MPI_LONG);
BOOST_MPI3_DECLARE_DATATYPE(unsigned long, MPI_UNSIGNED_LONG);
BOOST_MPI3_DECLARE_DATATYPE(float, MPI_FLOAT);
BOOST_MPI3_DECLARE_DATATYPE(double, MPI_DOUBLE);
BOOST_MPI3_DECLARE_DATATYPE(long double, MPI_LONG_DOUBLE);
BOOST_MPI3_DECLARE_DATATYPE(long long int, MPI_LONG_LONG_INT);

BOOST_MPI3_DECLARE_DATATYPE(cxx_float_complex, MPI_CXX_FLOAT_COMPLEX);
BOOST_MPI3_DECLARE_DATATYPE(cxx_double_complex, MPI_CXX_DOUBLE_COMPLEX);
BOOST_MPI3_DECLARE_DATATYPE(cxx_long_double_complex, MPI_CXX_DOUBLE_COMPLEX);

BOOST_MPI3_DECLARE_DATATYPE(float_float, MPI_CXX_FLOAT_COMPLEX);
BOOST_MPI3_DECLARE_DATATYPE(double_double, MPI_CXX_DOUBLE_COMPLEX);
BOOST_MPI3_DECLARE_DATATYPE(long_double_long_double, MPI_CXX_DOUBLE_COMPLEX);

BOOST_MPI3_DECLARE_DATATYPE(float_int, MPI_FLOAT_INT);
BOOST_MPI3_DECLARE_DATATYPE(long_int, MPI_LONG_INT);
BOOST_MPI3_DECLARE_DATATYPE(double_int, MPI_DOUBLE_INT);
BOOST_MPI3_DECLARE_DATATYPE(short_int, MPI_SHORT_INT);
BOOST_MPI3_DECLARE_DATATYPE(int_int, MPI_2INT);
BOOST_MPI3_DECLARE_DATATYPE(long_double_int, MPI_LONG_DOUBLE_INT);

//BOOST_MPI3_DECLARE_DATATYPE(std::intptr_t, MPI_AINT);
//BOOST_MPI3_DECLARE_DATATYPE(std::size_t, MPI_AINT);
BOOST_MPI3_DECLARE_DATATYPE(void*, MPI_AINT);

//BOOST_MPI3_DECLARE_DATATYPE(bool, MPI_INT);
BOOST_MPI3_DECLARE_DATATYPE(bool, MPI_CXX_BOOL);

// LB 
// UB

#undef BOOST_MPI3_DECLARE_DATATYPE

template<class T, class = decltype(basic_datatype<T>::value)>
std::true_type is_basic_aux(T const&);
std::false_type is_basic_aux(...);

template<class T> struct is_basic : decltype(is_basic_aux(T{})){};

}}}

#if _TEST_BOOST_MPI3_DETAIL_DATATYPE

#include<iostream>
#include<string>
#include<boost/type_index.hpp>

namespace mpi3 = boost::mpi3;
using std::cout;

int main(int argc, char* argv[]){

	auto i = MPI_THREAD_SINGLE;
	cout << boost::typeindex::type_id_runtime(i).pretty_name() << std::endl;

	static_assert( mpi3::detail::is_basic<int>{} );
	static_assert( mpi3::detail::is_basic<double>{} );
	static_assert( mpi3::detail::is_basic<mpi3::detail::float_int>{} );

	static_assert( not mpi3::detail::is_basic<std::string>{} );

}

#endif
#endif

