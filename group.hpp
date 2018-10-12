#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && mpic++ -std=c++14 -O3 -Wall -Wextra -fmax-errors=2 `#-Wfatal-errors` -D_TEST_MPI3_GROUP $0x.cpp -o $0x.x && time mpirun -n 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
//  (C) Copyright Alfredo A. Correa 2018.
#ifndef MPI3_GROUP_HPP
#define MPI3_GROUP_HPP

#include "../mpi3/detail/iterator_traits.hpp"
#include "../mpi3/detail/strided.hpp"
#include "../mpi3/equality.hpp"
#include "../mpi3/communicator.hpp"
#include "../mpi3/window.hpp"

#define OMPI_SKIP_MPICXX 1  // https://github.com/open-mpi/ompi/issues/5157
#include<mpi.h>

#include<algorithm> // copy_n
#include<numeric> // iota
#include<vector>

#if 0
#include "../mpi3/detail/basic_communicator.hpp"
#include "../mpi3/info.hpp"
#include "../mpi3/port.hpp"
#include "../mpi3/status.hpp"
#include "../mpi3/operation.hpp"
#include "../mpi3/handle.hpp"
#include "../mpi3/detail/datatype.hpp"
#include "../mpi3/communication_mode.hpp"

#include "../mpi3/message.hpp"
#include "../mpi3/request.hpp"
#include "../mpi3/generalized_request.hpp"
#include "../mpi3/type.hpp"

#include "../mpi3/detail/datatype.hpp"
#include "../mpi3/detail/iterator.hpp"

#include "../mpi3/detail/value_traits.hpp"
#include "../mpi3/detail/buffer.hpp"
//#include "../mpi3/detail/is_memcopyable.hpp"
#include "../mpi3/detail/strided.hpp"
#include "../mpi3/detail/package.hpp"

//#include "../mpi3/exception.hpp"



#include <boost/optional.hpp>
//#include <boost/range/irange.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_array.hpp>

#define BOOST_PACKAGE_ARCHIVE_SOURCE

#include <boost/archive/detail/common_oarchive.hpp>
#include <boost/archive/detail/common_iarchive.hpp>
#include <boost/archive/basic_streambuf_locale_saver.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/archive/detail/auto_link_archive.hpp>
#include <boost/archive/detail/abi_prefix.hpp> // must be the last header

#include <boost/serialization/item_version_type.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>

#include <boost/mpl/placeholders.hpp>

// use this to avoid need for linking
#ifdef _MAKE_BOOST_SERIALIZATION_HEADER_ONLY
#include "../mpi3/serialization_hack/archive_exception.cpp"
#include "../mpi3/serialization_hack/extended_type_info.cpp"
#include "../mpi3/serialization_hack/extended_type_info_typeid.cpp"
#include "../mpi3/serialization_hack/basic_archive.cpp"
#include "../mpi3/serialization_hack/basic_iarchive.cpp"
#include "../mpi3/serialization_hack/basic_oarchive.cpp"
#include "../mpi3/serialization_hack/basic_iserializer.cpp"
#include "../mpi3/serialization_hack/basic_oserializer.cpp"
#endif

#include "../mpi3/package_archive.hpp"

#include<numeric> // std::accumulate
#include<cassert>
#include<string>
#include<iostream>
#include<vector>
#include<map>
#include<iterator> // iterator_traits
#include<type_traits>
#include<limits>
#include<thread>
#endif

namespace boost{
namespace mpi3{

class group{
	MPI_Group impl_ = MPI_GROUP_NULL;
public:
	MPI_Group& operator&(){return impl_;}
	MPI_Group const& operator&() const{return impl_;}
	group() : impl_{MPI_GROUP_EMPTY}{}
	group(mpi3::communicator const& c){
		int s = MPI_Comm_group(&c, &impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot construct group"};
	}
	group(mpi3::window<> const& w){ // TODO window<void>
		int s = MPI_Win_get_group(&w, &impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot get group"};
	}
	group(group const& d) = delete;
/*		: group()
	{
		std::vector<int> v(d.size());
		std::iota(v.begin(), v.end(), 0);
		int s = MPI_Group_incl(d.impl_, v.size(), v.data(), &impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot copy construct"};
	}*/
	group(group&& g) : impl_{std::exchange(g.impl_, MPI_GROUP_EMPTY)}{}
	group& operator=(group const&) = delete;
	group& operator=(group&& other){
		swap(other);
		other.clear();
		return *this;
	}
	void swap(group& other){std::swap(impl_, other.impl_);}
	void clear(){
		if(impl_ != MPI_GROUP_EMPTY){
			int s = MPI_Group_free(&impl_);
			assert( s == MPI_SUCCESS ); // don't want to throw from ctor
		//	if(s != MPI_SUCCESS) throw std::runtime_error{"cannot free group"};
		}
		impl_ = MPI_GROUP_EMPTY;
	}
	~group(){clear();}
	group include(std::initializer_list<int> il){
		group ret;
		int s = MPI_Group_incl(impl_, il.size(), il.begin(), &ret.impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"rank not available"};
		return ret;
	}
	group exclude(std::initializer_list<int> il){
		group ret;
		int s = MPI_Group_excl(impl_, il.size(), il.begin(), &ret.impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"rank not available"};
		return ret;
	}
	int rank() const{
		int rank = -1; 
		int s = MPI_Group_rank(impl_, &rank);
		if(s != MPI_SUCCESS) throw std::runtime_error("rank not available");
		return rank;
	}
	bool root() const{return rank() == 0;}
	int size() const{
		int size = -1;
		int s = MPI_Group_size(impl_, &size);
		if(s != MPI_SUCCESS) throw std::runtime_error{"rank not available"};
		return size;
	}
	group sliced(int first, int last, int stride = 1) const{
		int ranges[][3] = {{first, last - 1, stride}};
		group ret;
		int s = MPI_Group_range_incl(impl_, 1, ranges, &ret.impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot slice"};
		return ret;
	}
	bool empty() const{return size() == 0;}
	friend mpi3::equality compare(group const& self, group const& other){
		int result;
		int s = MPI_Group_compare(self.impl_, other.impl_, &result);
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot compare"};
		return static_cast<boost::mpi3::equality>(result);
	}
	bool operator==(group const& other) const{
		mpi3::equality e = compare(*this, other); 
		return e == mpi3::identical or e == mpi3::congruent;
	}
	bool operator!=(group const& other) const{return not operator==(other);}
	bool friend is_permutation(group const& self, group const& other){
		return compare(self, other) != mpi3::unequal;
	}
	friend group set_intersection(group const& self, group const& other){
		group ret;
		int s = MPI_Group_intersection(self.impl_, other.impl_, &ret.impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot difference"};
		return ret;
	}
	friend group set_difference(group const& self, group const& other){
		group ret;
		int s = MPI_Group_difference(self.impl_, other.impl_, &ret.impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot difference"};
		return ret;
	}
	friend group set_union(group const& self, group const& other){
		group ret;
		int s = MPI_Group_union(self.impl_, other.impl_, &ret.impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot union"};
		return ret;
	}
	int translate_rank(int rank, group const& other) const{
		int out;
		int s = MPI_Group_translate_ranks(impl_, 1, &rank, other.impl_, &out);
		if(s != MPI_SUCCESS) throw std::runtime_error{"error translating"};
		return out;
	}
#if 0
//	group(communicator const& comm){MPI_Comm_group(&comm, &impl_);}
//	template<class ContiguousIntIterator>
//	group(group const& other, ContiguousIntIterator ranks_begin, std::size_t n){
//		MPI_Group_incl(other.impl_, n, ranks_begin, &impl_);
//	}
//	group(group const& other, std::vector<int> ranks){// : group(other, ranks.data(), ranks.size()){
//		MPI_Group_incl(other.impl_, ranks.size(), ranks.data(), &impl_);
//	}
//	bool is_null() const{return MPI_GROUP_NULL == impl_;}
//	operator bool() const{return not is_null();}
	

	std::vector<int> translate_ranks(std::vector<int> const& ranks1, group const& other) const{
		std::vector<int> ranks2(ranks1.size());
		int s = MPI_Group_translate_ranks(impl_, ranks1.size(), ranks1.data(), other.impl_, ranks2.data());
		if(s != MPI_SUCCESS) throw std::runtime_error{"cannot translate_ranks"};
		return ranks2;
	}
	std::vector<int> translate_ranks(group const& other) const{
		std::vector<int> ranks1(other.size());
		std::iota(ranks1.begin(), ranks1.end(), 0);
		return translate_ranks(ranks1, other);
	}
public:
	template<class Iterator, class Size>
	group include_n(Iterator it, Size count) const{
		return include_n_dispatch(typename std::iterator_traits<Iterator>::value_type{}, detail::iterator_category<Iterator>{}, it, count); 
	}
	template<class Iterator>
	group include(Iterator first, Iterator last) const{return include_n(first, std::distance(first, last));}
	template<class Range>
	group include(Range const& r) const{using std::begin; using std::end; return include(begin(r), end(r));}
private:
	template<class Iterator, class Size>
	group include_n_dispatch(int, detail::contiguous_iterator_tag, Iterator first, Size count) const{
		group ret;
		MPI_Group_incl(impl_, count, detail::data(first), &ret.impl_);
		return ret;
	}
	template<class Iterator, class Size, class Value, class = decltype(int(Value{}))>
	group include_n_dispatch(Value, std::input_iterator_tag, Iterator first, Size count) const{
		std::vector<int> v(count); std::copy_n(first, count, v.begin());
		return include_n(v.data(), v.size()); 
	}
public:
//	group include(std::vector<int> const& ranks){return group(*this, ranks);}
	template<class ContiguousIntIterator, class Size>
	group exclude_n(ContiguousIntIterator it, Size count) const{
		group ret;
		MPI_Group_excl(impl_, count, boost::mpi3::detail::data(it), &ret.impl_);
		return ret;
	}
	template<class ContiguousIntIterator>
	group exclude(ContiguousIntIterator first, ContiguousIntIterator last) const{
		return exclude_n(first, std::distance(first, last));
	}
	template<class ContiguousRangeIterator, class Size>
	group range_exclude_n(ContiguousRangeIterator it, Size n) const{
		group ret;
		using int3 = int[3];
		auto ptr = const_cast<int3*>(reinterpret_cast<int3 const*>(boost::mpi3::detail::data(it)));
		int status = MPI_Group_range_excl(impl_, n, ptr, &ret.impl_);
		if(status != MPI_SUCCESS) throw std::runtime_error("cannot exclude");
		return ret;
	}
	template<class ContiguousRangeIterator, class Size>
	group range_include_n(ContiguousRangeIterator it, Size n) const{
		group ret;
		using int3 = int[3];
		auto ptr = const_cast<int3*>(reinterpret_cast<int3 const*>(boost::mpi3::detail::data(it)));
		int status = MPI_Group_range_incl(impl_, n, ptr, &ret.impl_);
		if(status  != MPI_SUCCESS) throw std::runtime_error("cannot include");
		return ret;
	}
	template<class ContiguousRangeIterator>
	group range_exclude(ContiguousRangeIterator first, ContiguousRangeIterator last) const{
		return range_exclude_n(first, std::distance(first, last));
	}
//	group range_exclude(std::vector<boost::mpi3::strided_range> const& ranges){
//		return range_exclude(ranges.begin(), ranges.end());
//	}
	template<class ContiguousRangeIterator>
	group range_include(ContiguousRangeIterator first, ContiguousRangeIterator last) const{
		return range_include_n(first, std::distance(first, last));
	}
//	group range_include(std::vector<boost::mpi3::strided_range> const& ranges){
//		return range_include(ranges.begin(), ranges.end());
//	}
#endif
};

inline communicator communicator::create(struct group const& g) const{
	communicator ret;
	int s = MPI_Comm_create(impl_, &g, &ret.impl_);
	if(s != MPI_SUCCESS) throw std::runtime_error{"cannot crate group"};
	return ret;
}

inline communicator communicator::create_group(struct group const& g, int tag = 0) const{
	communicator ret;
	int s = MPI_Comm_create_group(impl_, &g, tag, &ret.impl_);
	if(s != MPI_SUCCESS) throw std::runtime_error{"cannot create_group"};
	return ret;
}

}}

#ifdef _TEST_MPI3_GROUP

#include "../mpi3/main.hpp"

#include<iostream>

using std::cout;
namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], mpi3::communicator world){
	mpi3::communicator w1 = world;
	assert( w1.size() == world.size() );
	assert( w1.rank() == world.rank() );
	mpi3::group g1 = w1;
	assert( g1.rank() == w1.rank() );
	mpi3::communicator w2 = w1.create(g1);
	assert( w2.size() == w1.size() );
	assert( w2.rank() == w1.rank() );
	assert( w2.rank() == world.rank() );
	return 0;
}

#endif
#endif


