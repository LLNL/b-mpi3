#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_SHM_MEMORY $0x.cpp -o $0x.x -lrt && time mpirun -np 2 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHM_MEMORY_HPP
#define BOOST_MPI3_SHM_MEMORY_HPP

#include "../../mpi3/shared_window.hpp"
#include<memory>

namespace boost{
namespace mpi3{
namespace shm{

struct shared_memory_object{
	communicator& comm_;
	std::unique_ptr<mpi3::shared_window> swUP_;
	shared_memory_object(communicator& c) : comm_(c){}
	shared_memory_object(shared_memory_object const&) = delete;
	void truncate(mpi3::size_t n){
		swUP_ = std::make_unique<mpi3::shared_window>(comm_.make_shared_window<char>(comm_.rank()==0?n:0));
	}
};

template<class T>
struct array_ptr{
	using value_type = T;
	shared_window* swP_;  // no shared, I want it to leak if not properly used
	std::ptrdiff_t offset_ = 0;
//	explicit operator T*(){return (T*)smo_.swP_->base(0) + offset_;}
	array_ptr<T>& operator++(){++offset_;}
	T& operator*() const{return *( (T*)swP_->base(0) + offset_);}
	T& operator[](int i) const{return *( (T*)swP_->base(0) + offset_ + i);}
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	array_ptr& operator=(array_ptr const& other) = default;
};

template<>
struct array_ptr<void>{
//	std::shared_ptr<shared_window> swSP_;
	shared_window* swP_; // no shared, I want it to leak if not properly used
	std::ptrdiff_t offset_ = 0;
	array_ptr(std::nullptr_t = nullptr){}
	array_ptr(array_ptr const& other) = default;
	template<class Other>
	array_ptr(array_ptr<Other> other) : swP_(other.swP_), offset_(other.offset_){}
	template<class Other>
	explicit operator array_ptr<Other>() const{array_ptr<Other> ret; ret.swP_ = swP_; ret.offset_ = offset_; return ret;}
	array_ptr& operator=(array_ptr const& other) = default;
};

template<class T> struct allocator;

struct managed_shared_memory{
	communicator& comm_;
	managed_shared_memory(communicator& comm) : comm_(comm){}
	array_ptr<void> allocate(mpi3::size_t n){
		array_ptr<void> ret;
		ret.swP_ = new shared_window(comm_.make_shared_window<char>(comm_.rank()==0?n:0)); //(comm_.rank()==0?n:0);
	//	ret.swSP_ = std::make_shared<shared_window>(comm_.make_shared_window<char>(comm_.rank()==0?n:0));
		return ret;
	}
	void deallocate(array_ptr<void> ptr){delete ptr.swP_;}
	template<class T>
	allocator<T> get_allocator();
};

template<class T>
struct allocator{
	managed_shared_memory& msm_;
	allocator(managed_shared_memory& msm) : msm_(msm){}
	array_ptr<T> allocate(mpi3::size_t n){
		return static_cast<array_ptr<T>>(msm_.allocate(n*sizeof(T)));
	}
	void deallocate(array_ptr<T> ptr){
		msm_.deallocate(ptr);//static_cast<array_ptr<void>>(ptr));
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
			mpi3::shm::allocator<int> a = mpi3mshm.get_allocator<int>();
			mpi3::shm::array_ptr<int> ptr = a.allocate(10);
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

	return 0;
}
#endif
#endif

