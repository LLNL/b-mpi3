#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++17 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_PROCESSOR_NAME -lboost_serialization -lboost_timer $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_PROCESSOR_HPP
#define BOOST_MPI3_PROCESSOR_HPP

#include<mpi.h>

#include "alf/boost/mpi3/communicator.hpp"
#include "alf/boost/mpi3/allocator.hpp"
#include <boost/container/vector.hpp>

namespace boost{
namespace mpi3{

template<class T>
struct reference;

template<class T>
class pointer{

	communicator comm_;
	int64_t n_;
	T* local_;

	pointer(communicator const& comm, int rank = 0, int64_t n = 1, T* local = nullptr) : comm_(comm), base_rank_(rank), n_(n), local_(local){}
public:
	pointer(){}
	pointer(std::nullptr_t) : comm_(communicator::null), rank_(0), n_(0), local_(nullptr){}
	pointer& operator=(pointer const& other){pointer p = other; swap(p); return *this;}
	void swap(pointer& p){
		std::swap(comm_, p.comm_);
		std::swap(rank_, p.rank_);
		std::swap(n_, p.n_);
		std::swap(local_, p.local_);
	}
	operator pointer<void const>() const{
		return {comm_, rank_, n_, static_cast<void const*>(local_)};
	}

	reference<T> operator*() const;
	reference<T> operator[](int64_t i) const;

	pointer operator+(int64_t d) const{
		pointer ret{comm_, 1};
		ret.local_ = nullptr;
		if(comm_.rank() == d/n) ret.local_;
		return {comm_, rank_ + d, n, local_ + d + ((rank_< d%comm_.size())?1:0)};
	}
	bool operator==(pointer const&) = default;
/*	bool operator==(pointer<T> const& other){
		return 
			comm_ == other.comm_ &&
			rank_ == other.rank_ &&
			local_ == other.local_
		;
	}*/
	auto operator-(pointer<T> other) const{
	//	assert(comm_ == other.comm_);
		assert(rank_ == other.rank_);
		return local_ - other.local_;
	}
	double* operator->() const{return local_;}
};


	std::size_t n_ = 1;


template<>
struct reference<double>{
	pointer<double> p_;
	reference() = delete;
/*	reference(double& d){
		if(p_.comm_.rank() == p_.rank_) *p_.local_ = d;
		p_.comm_.barrier();
	}*/
	operator double() const{
		double ret;
		if(p_.comm_.rank() == p_.rank_) ret = *p_.local_;
		p_.comm_.broadcast_n(&ret, 1, p_.rank_);
		return ret;
	};
	reference<double> operator=(double const& d){
		if(p_.comm_.rank() == p_.rank_) *p_.local_ = d;
		p_.comm_.barrier();
		return *this;
	}
};

template<class T>
reference<T> pointer<T>::operator*() const{
	return reference<double>{*this};
}

template<class T, class LocalAllocator = boost::mpi3::allocator<T>>
struct shared_allocator : LocalAllocator{

	using reference          = boost::mpi3::reference<T>;
	using pointer            = boost::mpi3::pointer<T>;
	using const_void_pointer = boost::mpi3::pointer<void const>;
	using void_pointer       = boost::mpi3::pointer<void>;
	using size_type          = typename boost::mpi3::allocator<T>::size_type;

	LocalAllocator alloc_;
	communicator& comm_;

	shared_allocator(communicator& comm, int N, LocalAllocator alloc = {}) : comm_(comm), N_(N), alloc_(alloc), rank_(0){}
	shared_allocator() = default;
	shared_allocator(shared_allocator const& other) = default;

	pointer allocate(size_type n){//, const void* hint = 0){
		pointer ret{comm_};
		auto local_size = n/comm_.size() + ((comm_.rank()<n%comm_.size())?1:0);
		if(local_size > 0) ret.local_ = alloc_.allocate(n);
		return ret;
	}
	void deallocate(pointer p, std::size_t n){
		auto local_size = n/p.comm_.size() + ((p.comm_.rank()<n%p.comm_.size())?1:0);
		if(local_size > 0) ret.local_ = alloc_.deallocate(p.local_, local_size);
	}
};

}}

/*
namespace __gnu_cxx{
	template<class T, class L> struct __alloc_traits<boost::mpi3::shared_allocator<T, L>> : std::allocator_traits<L>{
		using reference = boost::mpi3::reference<T>;
		using const_reference = boost::mpi3::reference<T>;
		using pointer = boost::mpi3::pointer<T>;
		using const_pointer = boost::mpi3::pointer<T>;
		template<class A>
		struct rebind{
			using other = typename boost::mpi3::shared_allocator<A>;//, typename L::rebind<A>::other>;
		};
	};
};*/

#ifdef _TEST_BOOST_MPI3_PROCESSOR_NAME

#include "alf/boost/mpi3/main.hpp"
#include<vector>
#include<typeinfo>
#include <boost/type_index.hpp>
#include <boost/container/vector.hpp>

using std::cout;
using std::endl;

#include<boost/container/vector.hpp>

using boost::container::vector;

struct animal{
	vector<animal> children;
};

int boost::mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){

	boost::mpi3::shared_allocator<double> salloc(world);

	boost::mpi3::pointer<double> p1 = salloc.allocate(std::size_t(1));
	boost::mpi3::pointer<double> p2 = salloc.allocate(std::size_t(1));
	boost::mpi3::pointer<double> p3 = salloc.allocate(std::size_t(1));
	boost::mpi3::pointer<double> p4 = salloc.allocate(std::size_t(1));


	boost::container::vector<double, boost::mpi3::shared_allocator<double>> v1(100, salloc);

//	bool& b = v1[2];

	return 0;

//	*p4 = 4.;
//	double d = *p4;
	if(world.rank() == 0){
//		std::cout << d << '\n';
	}
//	boost::container::vector<double, boost::mpi3::shared_allocator<double>> v1(100, salloc);
//	boost::container::vector<double, boost::mpi3::shared_allocator<double>> v2(100, salloc);

//	v1[5] = 41.;
//	std::cout << v1[5] << '\n';

//	v2[8] = 91.;
//	std::cout << boost::typeindex::type_id_with_cvr<decltype(v2[8])>() << '\n';

}

#endif
#endif

