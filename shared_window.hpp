#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_SHARED_WINDOW $0x.cpp -o $0x.x && time mpirun -np 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHARED_WINDOW_HPP
#define BOOST_MPI3_SHARED_WINDOW_HPP

#include "../mpi3/window.hpp"
#include "../mpi3/communicator.hpp"
#include<memory>

namespace boost{
namespace mpi3{

struct shared_window : window{
	shared_window(communicator& comm, mpi3::size_t n) : window{}{
		int disp_unit = 1;
		void* base_ptr = nullptr;
		int s = MPI_Win_allocate_shared(n, 1, MPI_INFO_NULL, comm.impl_, &base_ptr, &impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot create shared window");
	}
	mpi3::size_t size(int rank) const{
		MPI_Aint size;
		int disp_unit;
		void* baseptr;
		MPI_Win_shared_query(impl_, rank, &size, &disp_unit, &baseptr);
		return size;
	}
	mpi3::size_t disp_unit(int rank) const{
		MPI_Aint size;
		int disp_unit;
		void* baseptr;
		MPI_Win_shared_query(impl_, rank, &size, &disp_unit, &baseptr);
		return size;
	}
	void* base(int rank) const{
		MPI_Aint size;
		int disp_unit;
		void* baseptr;
		MPI_Win_shared_query(impl_, rank, &size, &disp_unit, &baseptr);
		return baseptr;
	}
};

template<class T /* = char*/> 
shared_window communicator::allocate_shared(
	mpi3::size_t size, 
	int disp_unit /* = 1*/
){
//	void* base_ptr;
	return shared_window(*this, size, disp_unit);
//	MPI_Win_allocate_shared(size, disp_unit, MPI_INFO_NULL, impl_, &base_ptr, &ret.impl_);
//	return ret;
}

namespace intranode{

template<class T> struct array_ptr;

template<>
struct array_ptr<const void>{
	using T = const void;
	std::shared_ptr<shared_window> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
};

template<>
struct array_ptr<void>{
	using T = void;
	std::shared_ptr<shared_window> wSP_;
	std::ptrdiff_t offset = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
};

template<class T>
struct array_ptr{
	std::shared_ptr<shared_window> wSP_;
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
	using value_type = T;
	using pointer = array_ptr<T>;
	using const_pointer = array_ptr<T const>;
	using reference = T&;
	using const_reference = T const&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	mpi3::communicator& comm_;
	allocator(mpi3::communicator& comm) : comm_(comm){}
	template<class U>
	allocator(allocator<U> const& other) : comm_(other.comm_){}

	array_ptr<T> allocate(size_type n, const void* hint = 0){
		array_ptr<T> ret;
		if(n == 0) return ret;
		ret.wSP_ = std::make_shared<shared_window>(
			comm_.allocate_shared(comm_.rank()==0?n*sizeof(T):1)
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
};

}


}}

#ifdef _TEST_BOOST_MPI3_SHARED_WINDOW

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/mutex.hpp"
#include "alf/boost/mpi3/mutex.hpp"
#include "alf/boost/mpi3/shm/vector.hpp"

#include<algorithm> // std::generate
#include<chrono>
#include<iostream>
#include<mutex>
#include<random>
#include<thread> 

int rand(int lower, int upper){
	static std::random_device rd;
	static std::mt19937 rng(rd());
	static std::uniform_int_distribution<int> uni(lower, upper); 
	return uni(rng);
}
int rand(int upper = RAND_MAX){return rand(0, upper);}

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::shm::vector<double> v(10, world);
	if(world.rank() == 0){
		for(int i = 0; i != 10; ++i){
			std::this_thread::sleep_for(std::chrono::milliseconds(rand(100)));
			v[i] = (i+1)*10;
		}
	}

	mpi3::mutex m(world);
	m.lock();
	if(world.rank() == 1)
		for(int i = 0; i != 10; ++i){
			v[i] = -(i+1)*10;
			std::this_thread::sleep_for(std::chrono::milliseconds(rand(1000)));
		}
	m.unlock();

	world.barrier();
	if(world.rank() == 2){
		for(int i = 0; i != 10; ++i)
			cout << v[i] << " " << std::flush;
		cout << '\n';
	}
	world.barrier();

	m.lock();
//	v.resize(3);
	if(world.rank() == 2)
		for(int i = 0; i != 3; ++i)
			cout << v[i] << " " << std::flush;

	m.unlock();

}

#endif
#endif

