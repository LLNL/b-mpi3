#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && mpic++ -O3 -std=c++14 -Wall -Wfatal-errors -D_TEST_BOOST_MPI3_SHARED_WINDOW $0x.cpp -o $0x.x && time mpirun -np 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHARED_WINDOW_HPP
#define BOOST_MPI3_SHARED_WINDOW_HPP

#include "../mpi3/shared_communicator.hpp"
#include "../mpi3/dynamic_window.hpp"

#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include<mpi.h>

namespace boost{
namespace mpi3{

template<class T /*= void*/>
struct shared_window : window<T>{
	shared_window(shared_communicator& comm, mpi3::size_t n, int disp_unit = sizeof(T)) : 
		window<T>()
	{
		void* base_ptr = nullptr;
		int s = MPI_Win_allocate_shared(n*sizeof(T), disp_unit, MPI_INFO_NULL, &comm, &base_ptr, &this->impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot create shared window");
	}
	shared_window(shared_communicator& comm, int disp_unit = sizeof(T)) : 
		shared_window(comm, 0, disp_unit)
	{}
	using query_t = std::tuple<mpi3::size_t, int, void*>;
	query_t query(int rank = MPI_PROC_NULL) const{
		query_t ret;
		MPI_Win_shared_query(this->impl_, rank, &std::get<0>(ret), &std::get<1>(ret), &std::get<2>(ret));
		return ret;
	}
	template<class TT = T>
	mpi3::size_t size(int rank = 0) const{
		return std::get<0>(query(rank))/sizeof(TT);
	}
	int disp_unit(int rank = 0) const{
		return std::get<1>(query(rank));
	}
	template<class TT = T>
	TT* base(int rank = 0) const{return static_cast<TT*>(std::get<2>(query(rank)));}
//	template<class T = char>
//	void attach_n(T* base, mpi3::size_t n){MPI_Win_attach(impl_, base, n*sizeof(T));}
};

#if 0
struct managed_shared_memory{
	shared_window<> sw_;
	managed_shared_memory(shared_communicator& c, int s) : sw_(c, c.rank()==0?s:0){}
//	struct segment_manager{};
	using segment_manager = boost::interprocess::segment_manager<char, boost::interprocess::rbtree_best_fit<boost::interprocess::mutex_family>, boost::interprocess::iset_index>;
	segment_manager sm_;
	managed_shared_memory::segment_manager* get_segment_manager(){
		return &sm_;
	}
};
#endif

template<class T /*= char*/> 
shared_window<T> shared_communicator::make_shared_window(
	mpi3::size_t size
){
	return shared_window<T>(*this, size);
}

template<class T /*= char*/>
shared_window<T> shared_communicator::make_shared_window(){
	return shared_window<T>(*this);//, sizeof(T));
}

namespace intranode{

template<class T> struct array_ptr;

template<>
struct array_ptr<const void>{
	using T = const void;
	std::shared_ptr<shared_window<>> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
	bool operator==(std::nullptr_t) const{return (bool)wSP_;}
	bool operator!=(std::nullptr_t) const{return not operator==(nullptr);}
};

template<>
struct array_ptr<void>{
	using T = void;
	std::shared_ptr<shared_window<>> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
	bool operator==(std::nullptr_t) const{return (bool)wSP_;}
	bool operator!=(std::nullptr_t) const{return not operator==(nullptr);}
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
	T& operator[](int idx) const{return ((T*)(wSP_->base(0)) + offset)[idx];}
	T* operator->() const{return (T*)(wSP_->base(0)) + offset;}
//	T* get() const{return wSP_->base(0) + offset;}
	explicit operator bool() const{return (bool)wSP_;}//.get();}
	bool operator==(std::nullptr_t) const{return (bool)wSP_;}
	bool operator!=(std::nullptr_t) const{return not operator==(nullptr);}
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
	allocator() = delete;
	~allocator() = default;
	allocator(allocator const& other) : comm_(other.comm_){
		std::cout << "popd size " << other.comm_.size() << '\n';
	}
	template<class U>
	allocator(allocator<U> const& other) : comm_(other.comm_){}

//	template<class ConstVoidPtr = const void*>
	array_ptr<T> allocate(size_type n, const void* hint = 0){
		std::cerr << "allocating " << n << std::endl; 
		std::cerr << " from rank " << comm_.rank() << std::endl;
		std::cerr << "active1 " << bool(comm_) << std::endl;
		std::cerr << "active2 " << bool(&comm_ == MPI_COMM_NULL) << std::endl;
		std::cerr << "size " << comm_.size() << std::endl;
		std::cout << std::flush;
		comm_.barrier();
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
		std::cout << "construct: I am " << comm_.rank() << std::endl;
		::new((void*)p) U(std::forward<Args>(args)...);
	}
	template< class U >	void destroy(U* p){
		std::cout << "destroy: I am " << comm_.rank() << std::endl;
		p->~U();
	}
};

struct is_root{
	shared_communicator& comm_;
	template<class Alloc>
	is_root(Alloc& a) : comm_(a.comm_){}
	bool root(){return comm_.root();}
};


}


}}

#ifdef _TEST_BOOST_MPI3_SHARED_WINDOW

#include "../mpi3/main.hpp"

namespace mpi3 = boost::mpi3; using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	double* p;
	double* b;
	std::cout << (p < b) << std::endl;

	mpi3::shared_communicator node = world.split_shared();
	mpi3::shared_window<int> win = node.make_shared_window<int>(node.root()?node.size():0);

	assert(win.base() != nullptr);
	assert(win.size() == node.size());

	win.base()[node.rank()] = node.rank() + 1;
	node.barrier();
	for(int i = 0; i != node.size(); ++i) assert(win.base()[i] == i + 1);

	return 0;
}

#endif
#endif

