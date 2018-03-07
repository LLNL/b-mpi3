#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"">$0x.cpp) && mpic++ -O3 -std=c++17 `#-Wfatal-errors` -D_BOOST_MPI3_MAIN_ENVIRONMENT -D_TEST_BOOST_MPI3_DETAIL_ITERATOR $0x.cpp -o $0x.x && time mpirun -n 1 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_DETAIL_ITERATOR_HPP
#define BOOST_MPI3_DETAIL_ITERATOR_HPP

#include "./just.hpp"

//#include "../../mpi3/allocator.hpp"

#include<boost/type_index.hpp>
#include<boost/container/vector.hpp>

#include<iterator>
#include<memory>
#include<vector>
#include<type_traits>

namespace boost{
namespace mpi3{
namespace detail{

template<class T> struct is_declared_contiguous : std::false_type{};
template<class T> struct is_contiguous;

template<
	class ContIt, 
	typename = std::enable_if_t<
		/**/std::is_convertible<ContIt, typename std::vector<std::decay_t<decltype(*std::declval<ContIt>())>>::const_iterator>{}
		or  std::is_convertible<ContIt, typename std::basic_string<std::decay_t<decltype(*std::declval<ContIt>())>>::const_iterator>{}
		or  std::is_convertible<ContIt, typename boost::container::vector<typename std::iterator_traits<ContIt>::value_type>::const_iterator>{}
		or	std::is_pointer<ContIt>{}
		or	std::is_constructible<typename std::iterator_traits<ContIt>::pointer, ContIt>{}
		or	std::is_convertible<ContIt, typename std::iterator_traits<ContIt>::pointer>{}
		or	is_declared_contiguous<ContIt>{}
	>
>
typename std::iterator_traits<ContIt>::pointer
data(ContIt const& it){return std::addressof(*it);}

template<class ContIt>
auto cdata(ContIt const& it)
->decltype(data(&static_cast<decltype(*it) const&>(*it))){
	return data(&static_cast<decltype(*it) const&>(*it));
}

}}}

#ifdef _TEST_BOOST_MPI3_DETAIL_ITERATOR

#include "../../mpi3/allocator.hpp"
#include "../../mpi3/main.hpp"

#include<boost/container/static_vector.hpp>
#include<boost/container/flat_set.hpp>
#include<boost/container/flat_map.hpp>
#include<boost/container/small_vector.hpp>
#include<boost/container/stable_vector.hpp>

#include<boost/range/adaptor/strided.hpp>

#include<boost/type_index.hpp>

#include<cassert>
#include<list>
#include<valarray>
#include<boost/array.hpp>
#include<array>
#include<set>
#include<queue>
#include<iostream>
#include<typeinfo>
#include<numeric>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int, char*[], mpi3::environment&){

	using boost::mpi3::detail::data;
	using boost::mpi3::detail::cdata;

	std::vector<double> v(30);
	v[0] = 10.;
	assert( *data(v.begin()) == 10. );

	std::array<int, 4> arr;
	arr[0] = 10.;
//	assert( *data(arr.begin()) == 10. );

	std::valarray<double> varr(30);
	varr[0] = 11.;
//	assert( *data(std::begin(varr)) == 11. );

	// boost::array::iterators are pointers
	boost::array<double, 5> barr;
	barr[0] = 12.;
//	assert( *data(std::begin(barr)) == 12. );

	std::set<double> s; s.insert(13.);
//	assert( *data(std::begin(s)) == 13. ); // data fails here

	{
		std::vector<double> v(30);
		v[0] = 10.;
		assert( *data(v.cbegin()) == 10. );
	}
	// std::array::iterators and valarray's are pointers
	{
		std::array<int, 4> arr;
		arr[0] = 10.;
		assert( *data(arr.cbegin()) == 10. );
		*data(arr.begin()) = 12.;
		assert( arr[0] == 12. );
	}
	{
		std::array<int, 4> arr;
		arr[0] = 10.;
		assert( *cdata(arr.begin()) == 10. );
	}
	{
		std::string s = "hola";
		assert( *data(s.begin()) == 'h' );
	}
	// all boost.containers iterator that are contiguous are particular cases of vector
	{
		boost::container::static_vector<double, 30> const sv(20);
		assert( *data(sv.begin()) == 0. );
	}
	{
		boost::container::flat_set<double> fs = {1.,2.,3.}; // flat_set::iterator is the same type as static_vector::iterator
		using boost::mpi3::detail::data;
		assert( data(fs.begin())[1] == 2. );
	}
	{
		boost::container::flat_map<double, int> fs = {{1., 10},{2.,20},{3.,30}}; // flat_set::iterator is the same type as static_vector::iterator
		using boost::mpi3::detail::data;
		cout << boost::typeindex::type_id_runtime(fs.begin()) << '\n';
		assert( data(fs.begin())[1] == (std::pair<double, int>(2., 20)) );
	}
	{
		boost::container::small_vector<double, 5> sv(3); sv[1] = 2.;// = {1.,2.,3.};
		using boost::mpi3::detail::data;
		assert( data(sv.begin())[1] == 2. );
	}
	{
		boost::container::stable_vector<double> v(10); v[1] = 5.;
		using boost::mpi3::detail::data;
	//	assert ( data(v.begin())[1] == 5. ); // stable vector is not contiguous
	}
	{
		std::vector<bool> v(30, false); v[2] = true;
		assert((v.begin())[2] == true);
	//	assert( data(v.begin())[1] == true ); // vector<bool>::iterator is not contiguous
	}
	{
		assert(boost::mpi3::detail::is_contiguous<std::vector<double>::iterator>{});
	}
	{
		std::vector<double> v(100); std::iota(v.begin(), v.end(), 0);
		boost::range_detail::strided_iterator<
			std::vector<double>::iterator, 
			boost::random_access_traversal_tag
		> sit(v.begin(), v.end(), 2);
		boost::range_detail::strided_iterator<
			std::vector<double>::iterator, 
			boost::random_access_traversal_tag
		> end(v.end(), v.end(), 2);

	//	for( ; sit != end; ++sit){
	//		std::cout << *sit << '\n';
	//	}
	}
	{
		std::vector<double, boost::mpi3::allocator<double>> v(100);
//		assert( detail::data(v.begin()) == &v[0] );
		cout << boost::mpi3::detail::is_contiguous<std::vector<double>::iterator>{} << '\n';
		cout << boost::mpi3::detail::is_contiguous<std::vector<double, mpi3::allocator<double>>::iterator>{} << '\n';
		cout << boost::mpi3::detail::is_contiguous<std::string::iterator>{} << '\n';
		cout << boost::mpi3::detail::is_basic<std::string::iterator::value_type>{} << '\n';
	}

}
#endif
#endif

