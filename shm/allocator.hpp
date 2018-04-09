#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && mpic++ -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_SHM_ALLOCATOR $0x.cpp -o $0x.x -lrt && time mpirun -np 2 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHM_ALLOCATOR_HPP
#define BOOST_MPI3_SHM_ALLOCATOR_HPP

#include "../../mpi3/shared_window.hpp"

namespace boost{
namespace mpi3{
namespace shm{

template<class T> struct array_ptr;

template<>
struct array_ptr<const void>{
	using T = const void;
	std::shared_ptr<shared_window<>> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
};

template<>
struct array_ptr<void>{
	using T = void;
	std::shared_ptr<shared_window<>> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
};

template<class T>
struct array_ptr{
	std::shared_ptr<shared_window<T>> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(){}
	array_ptr(std::nullptr_t){}
//	array_ptr(std::nullptr_t = nullptr) : offset(0){}
	array_ptr(array_ptr const& other) = default;
//	array_ptr(T* const& other = nullptr) : offset(0){}
//	array_ptr(T* const& other = nullptr) : offset(0){}
	array_ptr& operator=(array_ptr const& other) = default;
	T& operator*() const{return *((T*)(wSP_->base(0)) + offset);}
	T& operator[](mpi3::size_t idx) const{return ((T*)(wSP_->base(0)) + offset)[idx];}
	T* operator->() const{return (T*)(wSP_->base(0)) + offset;}
//	T* get() const{return wSP_->base(0) + offset;}
	explicit operator bool() const{return (bool)wSP_;}//.get();}
	operator array_ptr<void const>() const{
		array_ptr<void const> ret;
		ret.wSP_ = wSP_;
		return ret;
	}
	array_ptr operator+(std::ptrdiff_t d) const{
		array_ptr ret(*this);
		ret += d;
		return ret;
	}
	std::ptrdiff_t operator-(array_ptr other) const{
		return offset - other.offset;
	}
	array_ptr& operator--(){--offset; return *this;}
	array_ptr& operator++(){++offset; return *this;}
	array_ptr& operator-=(std::ptrdiff_t d){offset -= d; return *this;}
	array_ptr& operator+=(std::ptrdiff_t d){offset += d; return *this;}
	bool operator==(array_ptr<T> const& other) const{
		return wSP_->base(0) == other.wSP_->base(0) and offset == other.offset;
	}
	bool operator!=(array_ptr<T> const& other) const{return not((*this)==other);}
	bool operator<=(array_ptr<T> const& other) const{
		return wSP_->base(0) + offset <= other.wSP_->base(0) + other.offset;
	}
};

template<class T> struct allocator{
	template<class U> struct rebind{typedef allocator<U> other;};
	using value_type = T;
	using pointer = array_ptr<T>;
	using const_pointer = array_ptr<T const>;
	using reference = T&;
	using const_reference = T const&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	mpi3::shared_communicator& comm_;
	allocator(mpi3::shared_communicator& comm) : comm_(comm){}
	template<class U>
	allocator(allocator<U> const& other) : comm_(other.comm_){}

	array_ptr<T> allocate(size_type n, const void* hint = 0){
		array_ptr<T> ret;
		if(n == 0) return ret;
		ret.wSP_ = std::make_shared<shared_window<T>>(
			comm_.make_shared_window<T>(comm_.root()?n:0)
		//	comm_.allocate_shared(comm_.rank()==0?n*sizeof(T):1)
		);
		return ret;
	}
	void deallocate(array_ptr<T> ptr, size_type){
		ptr.wSP_.reset();
	}
//	void deallocate(double* const&, std::size_t&){}
	bool operator==(allocator const& other) const{
		return comm_ == other.comm_;
	}
	bool operator!=(allocator const& other) const{
		return not (other == *this);
	}
	template<class U, class... Args>
	void construct(U* p, Args&&... args){
		::new((void*)p) U(std::forward<Args>(args)...);
	}
	template< class U >	void destroy(U* p){
		p->~U();
	}
};
fdsfsd
struct is_root{
	shared_communicator& comm_;
	is_root(shm::allocator a) : comm_(a.comm_){}
	bool root(){return comm_.root();}
};

}}}

#ifdef _TEST_BOOST_MPI3_SHM_ALLOCATOR

#include "../../mpi3/main.hpp"
#include "../../mpi3/mutex.hpp"

#include<thread>
#include<mutex>

namespace mpi3 = boost::mpi3;
using std::cout; 

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	mpi3::shared_communicator node = world.split_shared();
	mpi3::shm::allocator<int> alloc(node);
	auto p = alloc.allocate(node.size());
	alloc.construct(&p[node.rank()], -node.rank());
	node.barrier();
	for(int i = 0; i != node.size(); ++i) assert(p[i] == -i);
	node.barrier();
	for(int i = 0; i != node.size(); ++i) assert(p[i] == -i);
	node.barrier();
	alloc.destroy(&p[node.rank()]);
	alloc.deallocate(p, node.size());

	return 0;
}
#endif
#endif

