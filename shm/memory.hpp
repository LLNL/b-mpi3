#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_SHM_MEMORY $0x.cpp -o $0x.x -lrt && time mpirun -np 2 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHM_MEMORY_HPP
#define BOOST_MPI3_SHM_MEMORY_HPP

#include <boost/pool/simple_segregated_storage.hpp>
#include "../../mpi3/shared_window.hpp"
#include<memory>


namespace boost{
namespace mpi3{
namespace shm{

struct shared_memory_object{
	shared_communicator& comm_;
	std::unique_ptr<mpi3::shared_window> swUP_;
	shared_memory_object(shared_communicator& c) : comm_(c){}
	shared_memory_object(shared_communicator& c, mpi3::size_t n) : comm_(c){
		truncate(n);
	}
	shared_memory_object(shared_memory_object const&) = delete;
	void truncate(mpi3::size_t n){
		swUP_ = std::make_unique<mpi3::shared_window>(comm_.make_shared_window<char>(comm_.rank()==0?n:0));
	}
};

template<class T>
struct array_ptr;

#if 0
template<>
struct array_ptr<double const>{
	using T = double;
	using value_type = T;
	using reference = double const&;
	shared_window* swP_;  // no shared, I want it to leak if not properly used
	std::ptrdiff_t offset_ = 0;
//	explicit operator T*(){return (T*)smo_.swP_->base(0) + offset_;}
	array_ptr& operator++(){++offset_; return *this;}
	array_ptr& operator+=(mpi3::size_t n){offset_+=n; return *this;}
	array_ptr operator+(mpi3::size_t n) const{array_ptr ret(*this); return ret+=n;}
	mpi3::size_t operator-(array_ptr const& other) const{
		assert(swP_ == other.swP_);
		return offset_ - other.offset_;
	}
	T const* operator->() const{return (T*)swP_->base(0) + offset_;}
	T const& operator*() const{return *( (T*)swP_->base(0) + offset_);}
	T const& operator[](int i) const{return *( (T*)swP_->base(0) + offset_ + i);}
	operator double const*() const{return ( (T const*)swP_->base(0) + offset_);}

	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
	bool operator==(array_ptr const& other) const{return offset_ == other.offset_ and swP_ == other.swP_;}
	bool operator!=(array_ptr const& other) const{return (*this)==other;}
};
#endif

template<class T>
struct array_ptr{
	using value_type = T;
	using reference = T&;
	shared_window* swP_ = nullptr;  // no shared, I want it to leak if not properly used
	std::ptrdiff_t offset_ = 0;
//	explicit operator T*(){return (T*)smo_.swP_->base(0) + offset_;}
//	array_ptr<double>(double* const){}
//	array_ptr(nullptr_t){}
//	array_ptr(void*){}
//	array_ptr(T* p){} // this is necessary to allow the code triggered by boost.container::resize, should be harmless in that context
	array_ptr(){}
	array_ptr& operator++(){++offset_; return *this;}
	array_ptr& operator--(){--offset_; return *this;}
	array_ptr& operator+=(mpi3::size_t n){offset_+=n; return *this;}
	array_ptr& operator-=(mpi3::size_t n){offset_-=n; return *this;}
	friend array_ptr operator+(array_ptr p, mpi3::size_t n){return p+=n;}
	friend array_ptr operator-(array_ptr p, mpi3::size_t n){return p-=n;}
	mpi3::size_t operator-(array_ptr const& other) const{
//		assert(swP_ == other.swP_);
		return offset_ - other.offset_;
	}
	T* operator->() const{
		return swP_?((T*)(swP_->base(0)) + offset_):nullptr;
		if(swP_ == nullptr){
			if(offset_ == 0) return nullptr;
			else return offset_;
		}
	//	if(not swP_) return nullptr;
	//	return (T*)(swP_->base(0)) + offset_;
	}
	reference operator*() const{return *( (T*)swP_->base(0) + offset_);}
	reference operator[](mpi3::size_t i) const{return *( (T*)swP_->base(0) + offset_ + i);}
	operator array_ptr<T const>() const{array_ptr<T const> ret; ret.swP_ = swP_; return ret;}
	operator T*() const{
		return swP_?((T*)swP_->base(0) + offset_):nullptr;
	//	if(not swP_) return nullptr;
	//	return ( (T*)swP_->base(0) + offset_);
	}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
/*	array_ptr& operator=(T const* p){
		if(not p){
			swP_ = nullptr; offset_ = 0;
		}else{
			if(swP_) offset_ = p - (T*)swP_->base(0);
			else assert(0);
		}  
		return *this;
	}*/
//	array_ptr& operator=(T const* p){
//		assert(0);
//	}

//	array_ptr& operator=(nullptr_t){swP_=nullptr; offset_ = 0; return *this;}
	operator bool() const{return swP_;}
	bool operator==(array_ptr const& other) const{return offset_==other.offset_ and swP_==other.swP_;}
	bool operator!=(array_ptr const& other) const{return not ((*this)==other);}
};

template<>
struct array_ptr<void>{
//	std::shared_ptr<shared_window> swSP_;
	shared_window* swP_; // no shared, I want it to leak if not properly used
	std::ptrdiff_t offset_ = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	template<class Other>
	array_ptr(array_ptr<Other> other) : swP_(other.swP_), offset_(other.offset_*sizeof(Other)){}
	template<class Other>
	explicit operator array_ptr<Other>() const{array_ptr<Other> ret; ret.swP_ = swP_; ret.offset_ = offset_; return ret;}
	array_ptr& operator=(array_ptr const& other) = default;
};

template<>
struct array_ptr<void const>{
//	std::shared_ptr<shared_window> swSP_;
	shared_window* swP_; // no shared, I want it to leak if not properly used
	std::ptrdiff_t offset_ = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	template<class Other>
	array_ptr(array_ptr<Other> other) : swP_(other.swP_), offset_(other.offset_*sizeof(Other)){}
	template<class Other>
	explicit operator array_ptr<Other>() const{array_ptr<Other> ret; ret.swP_ = swP_; ret.offset_ = offset_; return ret;}
	array_ptr& operator=(array_ptr const& other) = default;
};

template<class T> struct allocator;

struct managed_shared_memory{
	shared_communicator& comm_;
	shared_window sw_;
	boost::simple_segregated_storage<std::size_t> storage_;
	managed_shared_memory(shared_communicator& comm, mpi3::size_t n) : 
		comm_(comm), 
		sw_(comm.make_shared_window<char>(comm.rank()?0:n))
	{
	//	if(comm_.rank()==0) 
		storage_.add_block(sw_.base(0), sw_.size(0), n);
	}
	void* malloc(){return storage_.malloc();}
#if 0
	array_ptr<void> allocate(mpi3::size_t n){
		array_ptr<void> ret;
		ret.swP_ = new shared_window(comm_.make_shared_window<char>(comm_.rank()==0?n:0)); //(comm_.rank()==0?n:0);
	//	ret.swSP_ = std::make_shared<shared_window>(comm_.make_shared_window<char>(comm_.rank()==0?n:0));
		return ret;
	}
	void deallocate(array_ptr<void> ptr){
		if(not ptr.swP_) return;
		delete ptr.swP_;
	}
	template<class T>
	allocator<T> get_allocator();
#endif
//	template<class T, class... Args>
//	T* construct(Args&&... args){
//		if(comm_.rank()==0){
//			T* p = storage_.construct<T>(std::forward<Args>(args)...);
//		}
//	}
	
};

template<class T>
struct allocator{
	using value_type = T;
	using pointer = array_ptr<T>;
	using const_pointer = array_ptr<T const>;
	using void_pointer = array_ptr<void>;
	using const_void_pointer = array_ptr<void const>;
	using size_type = mpi3::size_t;
	using difference_type = mpi3::size_t;
	managed_shared_memory& msm_;
	allocator(managed_shared_memory& msm) : msm_(msm){}
//	array_ptr<T> allocate(mpi3::size_t n, void const* hint = 0){
//		return static_cast<array_ptr<T>>(msm_.allocate(n*sizeof(T)));
//	}
//	void deallocate(array_ptr<T>& ptr, mpi3::size_t = 1){
//		msm_.deallocate(ptr);//static_cast<array_ptr<void>>(ptr));
//	}
	array_ptr<T> allocate(mpi3::size_t n){
		std::cout << "allocate-> " << n << std::endl;
		auto ret = msm_.allocate(n*sizeof(T));
		std::cout << "<-allocate " << n << std::endl;
		return static_cast<array_ptr<T>>(ret);
	//	return static_cast<array_ptr<T>>(msm_.allocate(n*sizeof(T)));
	}
	void deallocate(array_ptr<T> ptr, mpi3::size_t = 0){
		std::cout << "deallocate->" << std::endl;
		msm_.deallocate(ptr);//static_cast<array_ptr<void>>(ptr));
		std::cout << "<-deallocate" << std::endl;
	}
};

template<class T>
allocator<T> managed_shared_memory::get_allocator(){
	return allocator<T>(*this);
}

struct mapped_region{
	shared_memory_object& smo_;
	mapped_region(shared_memory_object& smo) : smo_(smo){}
	mapped_region(mapped_region const&) = delete;
	void* get_address(){return smo_.swUP_->base(0);}
	mpi3::size_t get_size(){return smo_.swUP_->size(0);}
//	mpi3::size_t get_free_memory() const;
//	void zero_free_memory() const;
//	bool all_memory_deallocated();
//	bool check_sanity();
};

}}}

#ifdef _TEST_BOOST_MPI3_SHM_MEMORY

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/mutex.hpp"

namespace mpi3 = boost::mpi3;
using std::cout; 

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	{
		mpi3::shm::shared_memory_object mpi3shm(world);
		mpi3shm.truncate(100);
		mpi3::shm::mapped_region mpi3region(mpi3shm);
		if(world.rank() == 0){
			std::memset(mpi3region.get_address(), 1, mpi3region.get_size());
		}
		world.barrier();
		if(world.rank() == 1){
			char* mem = static_cast<char*>(mpi3region.get_address());
			for(int i = 0; i != mpi3region.get_size(); ++i) assert(mem[i] == 1);
		}
	}
	{
		mpi3::shm::managed_shared_memory mpi3mshm(world);

		mpi3::shm::array_ptr<void> ptr = mpi3mshm.allocate(100);
		mpi3mshm.deallocate(ptr);

		ptr = mpi3mshm.allocate(200);
		mpi3mshm.deallocate(ptr);

		{
			mpi3::shm::allocator<double> a = mpi3mshm.get_allocator<double>();
			mpi3::shm::array_ptr<double> ptr = a.allocate(10);
			if(world.rank() == 0) std::fill_n(ptr, 10, 5);
			world.barrier();
			if(world.rank() == 1) for(int i = 0; i != 10; ++i) assert(ptr[i] == 5);
			a.deallocate(ptr);
		}
		{
			std::allocator<int> a;
			int* ptr = a.allocate(100);
			a.deallocate(ptr, 100);
		}
	}

	world.barrier();

	{
		mpi3::shm::managed_shared_memory mpi3mshm(world);
		std::atomic<int>& atomic = *mpi3mshm.construct<std::atomic<int>>(0);
	}

	return 0;
}
#endif
#endif

