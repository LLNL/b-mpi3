#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_DETAIL_PACKAGE_OARCHIVE $0x.cpp -o $0x.x -lboost_serialization && time mpirun -np 2 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif

#ifndef BOOST_MPI3_DETAIL_PACKAGE_OARCHIVE_HPP
#define BOOST_MPI3_DETAIL_PACKAGE_OARCHIVE_HPP


#include <sstream>

#include <boost/static_assert.hpp>
#include <boost/type_traits/is_array.hpp>
//#include <boost/serialization/pfto.hpp>

#define BOOST_PACKAGE_ARCHIVE_SOURCE

//#include <boost/archive/binary_oarchive_impl.hpp>
//#include <boost/archive/binary_iarchive_impl.hpp>

// include template definitions for base classes used.  Otherwise
// you'll get link failure with undefined symbols
//#include <boost/archive/impl/basic_binary_oprimitive.ipp>
//#include <boost/archive/impl/basic_binary_iprimitive.ipp>

//#include <boost/archive/impl/basic_binary_oarchive.ipp>
//#include <boost/archive/impl/basic_binary_iarchive.ipp>

//#include <boost/archive/impl/archive_pointer_iserializer.ipp>
//#include <boost/archive/impl/archive_pointer_oserializer.ipp>

#include "../../mpi3/package.hpp"

#include <boost/archive/detail/common_oarchive.hpp>
#include <boost/archive/detail/common_iarchive.hpp>
#include <boost/serialization/item_version_type.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>

#include <boost/archive/basic_streambuf_locale_saver.hpp>
#include <boost/archive/archive_exception.hpp>
#include <boost/serialization/is_bitwise_serializable.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/serialization/array.hpp>
#include <boost/archive/detail/auto_link_archive.hpp>
#include <boost/archive/detail/abi_prefix.hpp> // must be the last header

// use this to avoid need for linking
#ifdef MAKE_BOOST_SERIALIZATION_HEADER_ONLY
#include "../../mpi3/serialization_hack/archive_exception.cpp"
#include "../../mpi3/serialization_hack/extended_type_info.cpp"
#include "../../mpi3/serialization_hack/extended_type_info_typeid.cpp"
#include "../../mpi3/serialization_hack/basic_archive.cpp"
#include "../../mpi3/serialization_hack/basic_iarchive.cpp"
#include "../../mpi3/serialization_hack/basic_oarchive.cpp"
#include "../../mpi3/serialization_hack/basic_iserializer.cpp"
#include "../../mpi3/serialization_hack/basic_oserializer.cpp"
#endif

namespace boost{
namespace mpi3{
namespace detail{

class basic_package_iprimitive{
protected:
	boost::mpi3::package& p_;
public:
	// we provide an optimized save for all fundamental types
	// typedef serialization::is_bitwise_serializable<mpl::_1> 
	// use_array_optimization;
	// workaround without using mpl lambdas
	struct use_array_optimization {
		template<class T>  
		struct apply : 
			public mpl_::bool_<boost::mpi3::detail::is_builtin_type<T>::value>
		//	public boost::serialization::is_bitwise_serializable< T > 
		{};  
	};
	template<class T>
	void load_array(boost::serialization::array<T>& t, unsigned int = 0){
		p_.unpack_n(t.address(), t.count()); 
	}
    template<class T>
    void load(T& t){
		p_ >> t;
	} //	os_ += sizeof(T); //os << t;}
	basic_package_iprimitive(boost::mpi3::package& p) : p_(p){}
};


class basic_package_oprimitive{
protected:
	boost::mpi3::package& p_;
public:
	struct use_array_optimization {
		template <class T>
		struct apply : public boost::serialization::is_bitwise_serializable< T > {};  
	};
    template<class T>
    void save(const T& t, unsigned int = 0){
		p_ << t; //	os_ += sizeof(T); //os << t;
    }
	template<class T>
	void save_array(boost::serialization::array<T> const& t, unsigned int = 0){
		p_.pack_n(t.address(), t.count()); 
	}

//	template<class T>
//	void save(boost::serialization::array<T> const& t, int = 0){
//		assert(0);
//	}
//	void save(const boost::archive::object_id_type&){}
//	void save(const boost::archive::object_reference_type&){}
//	void save(const boost::archive::class_id_type&){}
//	void save(const boost::archive::class_id_optional_type&){}
//	basic_memory_oprimitive(size_t& os) : os_(os){}
	basic_package_oprimitive(boost::mpi3::package& p) : p_(p){}
};

template<class Archive>
class basic_package_iarchive : public boost::archive::detail::common_iarchive<Archive>{
    friend class boost::archive::detail::interface_iarchive<Archive>;
    typedef boost::archive::detail::common_iarchive<Archive> detail_common_iarchive;
    template<class T>
    void load_override(T& t, /*BOOST_PFTO*/ int = 0){
        this->detail_common_iarchive::load_override(t);//, 0);
    }
	protected:
	basic_package_iarchive(unsigned int flags) : boost::archive::detail::common_iarchive<Archive>(flags){}
};

template<class Archive>
class basic_package_oarchive : public boost::archive::detail::common_oarchive<Archive>{
    friend class boost::archive::detail::interface_oarchive<Archive>;
    typedef boost::archive::detail::common_oarchive<Archive> detail_common_oarchive;
protected:
    template<class T>
    void save_override(T& t, /*BOOST_PFTO*/ int = 0){
        this->detail_common_oarchive::save_override(t);//, 0);
    }


// metadata is ignored 
//   void save_override(const object_id_type&, int){/* this->This()->newline(); this->detail_common_oarchive::save_override(t, 0);*/}
//    void save_override(const class_id_optional_type&, int){}
//    void save_override(const class_name_type&, int){/*  const std::string s(t); * this->This() << s;*/}
	protected:
	basic_package_oarchive(unsigned int flags) : boost::archive::detail::common_oarchive<Archive>(flags){}
};

template<class Archive>
class package_iarchive_impl : public basic_package_iprimitive, public basic_package_iarchive<Archive>{
	public:



    template<class T>
    void load(T& t){basic_package_iprimitive::load(t);}
	// empty functions follow, so that metadata is not counted as part of the memory size
    void load(boost::archive::version_type&){}
//	void save(const boost::serialization::item_version_type&){/*save(static_cast<const unsigned int>(t));*/}
    void load(boost::archive::tracking_type&){/*save(static_cast<const unsigned int>(t));*/}

	void load(boost::archive::object_id_type&){}
	void load(boost::archive::object_reference_type&){}
	void load(boost::archive::class_id_type&){}
	void load(boost::archive::class_id_optional_type&){}
	void load(boost::archive::class_id_reference_type&){}
	void load(boost::archive::class_name_type&){}

	void load(boost::serialization::collection_size_type& t){
		unsigned int x = 0;
		load(x);
		t = serialization::collection_size_type(x);
	}
	void load(boost::serialization::item_version_type&){}

	void load(char * s){
		assert(0);
		const std::size_t len = std::ostream::traits_type::length(s);
		*this->This() << len;
	//	++tokens_;//	this->This()->newtoken();
	//	os_ += len*sizeof(char);//	os << s;
		p_.pack_n(s, len);
	}
	void load(wchar_t * ws){
		const std::size_t l = std::wcslen(ws);
		*this->This() << l;
		assert(0);
	//	++tokens_; // this->This()->newtoken();
	//	os_ += l*sizeof(wchar_t);//	os.write((const char *)ws, l * sizeof(wchar_t)/sizeof(char));
	}
	void load(std::string &s){
	//	const std::size_t size = s.size();
	//	*this->This() << size;
		std::size_t size; //  *this->This() >> size;
		p_.unpack_n(&size, 1);
		std::cout << " size = " << size << '\n';
		s.resize(size);
	//	++tokens_; // this->This()->newtoken();
	//	os_ += s.size()*sizeof(char);//	os << s;
		p_.unpack_n(const_cast<char*>(s.c_str()), size);
		std::cout << "unpacked string is " << s << '\n';
	}
	void load(std::wstring &ws){
    	const std::size_t size = ws.size();
		*this->This() << size;
	//	++tokens_; //	this->This()->newtoken();
	//	os_ += ws.size()*sizeof(wchar_t);//	os << s;
		assert(0);
	}
	public:
    package_iarchive_impl(boost::mpi3::package& p, unsigned int flags) : // size_t& os, size_t& tokens, unsigned int flags) :
		basic_package_iprimitive(p),
		basic_package_iarchive<Archive>(flags)
	{}
};


template<class Archive>
class package_oarchive_impl : public basic_package_oprimitive, public basic_package_oarchive<Archive>{
//	size_t& tokens_;
	public:
    template<class T>
    void save(const T& t){
//		std::cout << "saving " << typeid(T).name() << "\n";
	//	++tokens_; //this->newtoken();
        basic_package_oprimitive::save(t);
    }
	void save(boost::serialization::array<double>& t){
		assert(0);
	}
	// empty functions follow, so that metadata is not counted as part of the memory size
    void save(const boost::archive::version_type&){/*save(static_cast<const unsigned int>(t));*/}
//	void save(const boost::serialization::item_version_type&){/*save(static_cast<const unsigned int>(t));*/}
    void save(const boost::archive::tracking_type&){/*save(static_cast<const unsigned int>(t));*/}

	void save(const boost::archive::object_id_type&){}
	void save(const boost::archive::object_reference_type&){}
	void save(const boost::archive::class_id_type&){}
	void save(const boost::archive::class_id_optional_type&){}
	void save(const boost::archive::class_id_reference_type&){}
	void save(const boost::archive::class_name_type&){}

	void save(const boost::serialization::collection_size_type& t){save(static_cast<const unsigned int>(t));}
	void save(const boost::serialization::item_version_type&){}


	// string types (like char*, string, etc) have special handling
	// types that need special handling
	void save(const char * s){
		assert(0);
		const std::size_t len = std::ostream::traits_type::length(s);
		*this->This() << len;
	//	++tokens_;//	this->This()->newtoken();
	//	os_ += len*sizeof(char);//	os << s;
		p_.pack_n(s, len);
	}
	void save(const wchar_t * ws){
		const std::size_t l = std::wcslen(ws);
		*this->This() << l;
		assert(0);
	//	++tokens_; // this->This()->newtoken();
	//	os_ += l*sizeof(wchar_t);//	os.write((const char *)ws, l * sizeof(wchar_t)/sizeof(char));
	}
	void save(const std::string &s){
    	const std::size_t size = s.size();
	//	*this->This() << size;
		p_.pack_n(&size, 1);
		std::cout << " packed size = " << size << '\n';
	//	++tokens_; // this->This()->newtoken();
	//	os_ += s.size()*sizeof(char);//	os << s;
		p_.pack_n(s.c_str(), size);
	}
	void save(const std::wstring &ws){
    	const std::size_t size = ws.size();
		*this->This() << size;
	//	++tokens_; //	this->This()->newtoken();
	//	os_ += ws.size()*sizeof(wchar_t);//	os << s;
		assert(0);
	}

//	using package_oarchive_impl<package_oarchive>::save_override;

#if 1
	// Save all supported datatypes directly
	template<class T>
	void save(
		boost::serialization::array<T> const& t, unsigned int
	)
//	T const& x)
  {
		assert(0);
//    typedef typename mpl::apply1<use_array_optimization,T>::type use_optimized;
		save_override(t, boost::mpl::bool_<true>{});//std::true_type{});
//    save_override(x, use_optimized());
  }
#endif

	public:
    package_oarchive_impl(boost::mpi3::package& p, unsigned int flags) : // size_t& os, size_t& tokens, unsigned int flags) :
		basic_package_oprimitive(p),
		basic_package_oarchive<Archive>(flags)
	{}
};

struct package_iarchive : public package_iarchive_impl<package_iarchive>{
    package_iarchive(boost::mpi3::package& p, unsigned int flags = 0) 
    : package_iarchive_impl<package_iarchive>(p, flags){}
};

struct package_oarchive : public package_oarchive_impl<package_oarchive>{
    package_oarchive(boost::mpi3::package& p, unsigned int flags = 0) 
    : package_oarchive_impl<package_oarchive>(p, flags){}

	using package_oarchive_impl<package_oarchive>::operator&;
//	template<class T>
	package_oarchive& operator & (boost::serialization::array<double>& t){
		assert(0);
		return *this;
	}

};

//}}}
}}}

//BOOST_SERIALIZATION_REGISTER_ARCHIVE(boost::mpi3::package_oarchive)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(boost::mpi3::detail::package_oarchive)
BOOST_SERIALIZATION_USE_ARRAY_OPTIMIZATION(boost::mpi3::detail::package_iarchive)

namespace boost{
namespace mpi3{

template<class CommunicationMode, class ContiguousIterator, class Size, class ValueType = typename std::iterator_traits<ContiguousIterator>::value_type>
void communicator::send_n_randomaccess_contiguous_builtin(CommunicationMode cm, blocking_mode, std::false_type, ContiguousIterator first, Size n, int dest, int tag){
	auto p = make_package();
	detail::package_oarchive poa(p);
	while( n ){
		poa << *first;
		++first;
		--n;
	}
	p.send(dest, tag);
//	return first
}

template<class CommunicationMode, class ContiguousIterator, class Size, class ValueType = typename std::iterator_traits<ContiguousIterator>::value_type>
void communicator::receive_n_randomaccess_contiguous_builtin(CommunicationMode cm, blocking_mode, std::false_type, ContiguousIterator first, Size n, int dest, int tag){
//	assert(0);
	auto p = make_package();
	p.receive(dest, tag);
	detail::package_iarchive pia(p);
	while(n){
		pia >> *first;
		++first;
		--n;
	}
}

template<class CommunicationMode, class BlockingMode, class InputIterator>
auto communicator::send_category(CommunicationMode cm, BlockingMode bm, std::input_iterator_tag, 
	InputIterator first, InputIterator last, int dest, int tag
){
	auto p = make_package();
	detail::package_oarchive poa(p);
	for( ; first != last; ++first){
		poa << *first;
	}
	p.send(dest, tag);
}

template<class ContiguousIt, typename Size>
void communicator::broadcast_n_contiguous_builtinQ(std::false_type, ContiguousIt first, Size count, int root){
	auto p = make_package();
	if(rank() == root){
		detail::package_oarchive poa(p);
		while(count){
			poa << *first;
			++first;
			--count;
		}
	}
	p.broadcast(root);
	if(rank() != root){
		detail::package_iarchive pia(p);
		while(count){
			pia >> *first;
			++first;
			--count;
		}
	}
}


}}

#ifdef _TEST_BOOST_MPI3_DETAIL_PACKAGE_OARCHIVE

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/process.hpp"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>


namespace mpi3 = boost::mpi3;
using std::cout; 

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	if(world.rank() == 0){
		auto p = world.make_package();
		int i = 12;
		int j = 13;
		boost::mpi3::detail::package_oarchive poa(p);
		std::string s("hello");
		poa << s;
		poa << i;
		poa << j;
		std::vector<double> v(20, 5.);
		poa << v;
	//	cout << "size " << p.size() << '\n';
		cout << "size " << p.size() << '\n';
		poa << 5;
		std::map<int, int> m = {{1,2},{2,4},{3,4}};
		poa << m;
		p.send(1);
	}else if(world.rank() == 1){
		auto p = world.make_package();
		p.receive(0);
		boost::mpi3::detail::package_iarchive pia(p);
		std::string s;
		pia >> s;
		cout << "s = " << s << '\n';
		assert( s == "hello" );
		int i = 0;
		int j = 0;
		pia >> i;
		pia >> j;
		assert( i == 12 );
		assert( j == 13 );
		std::vector<double> v;
		pia >> v;
		std::cout << "v size " << v.size() << '\n';
		assert(v.size() == 20);
		int c;
		pia >> c;
		assert(c == 5);
		std::map<int, int> m;
		pia >> m;
		assert( m[3] == 4 );
	}

	if(world.rank() == 0){
		auto p = world.make_package();
		boost::mpi3::detail::package_oarchive poa(p);
		std::vector<double> v = {12.,13};
		poa << v;
		p.send(1);
	}else if(world.rank() == 0){
		int i = 0, j = 0;
		world[0] >> i;
		world[0] >> j;
		assert( i != 12 );
		assert( j != 13 );
	}

	return 0;
}

#endif
#endif

