#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && time mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_SHM_MANAGED_SHARED_MEMORY $0x.cpp -o $0x.x -lrt -lboost_system && time mpirun -np 6 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif

#ifndef BOOST_MPI3_SHM_MANAGED_SHARED_MEMORY_HPP
#define BOOST_MPI3_SHM_MANAGED_SHARED_MEMORY_HPP

#include <boost/pool/simple_segregated_storage.hpp>
#include "../../mpi3/shared_window.hpp"
#include<random>
#include<thread>

int rand(int lower, int upper){
	static std::random_device rd;
	static std::mt19937 rng(rd());
	static std::uniform_int_distribution<int> uni(lower, upper); 
	return uni(rng);
}
int rand(int upper = RAND_MAX){return rand(0, upper);}

namespace boost{
namespace mpi3{
namespace shm{

struct managed_shared_memory{
	communicator& comm_;
	shared_window sw_;
	using size_type = mpi3::size_t;
	using difference_type = mpi3::size_t;
	boost::simple_segregated_storage<std::size_t> storage_;
	managed_shared_memory(communicator& comm, mpi3::size_t n) : 
		comm_(comm), 
		sw_(comm.make_shared_window<char>(comm.rank()?0:n))
	{
		storage_.add_block(sw_.base(0), sw_.size(0), n);
	}
	template<class T>
	T* malloc(){
		return (T*)storage_.malloc_n(1, sizeof(T));
	}
	template<class T>
	T* malloc_n(mpi3::size_t n){
		return (T*)storage_.malloc_n(1, n*sizeof(T));
	}
	template<class T>
	void free_n(T* ptr, mpi3::size_t n){
		storage_.free_n(ptr, 1, n*sizeof(T));
	}
	template<class T, class... Args>
	T* construct(Args&&... args){
		sw_.lock_all();//exclusive(0);
		sw_.sync();
		T* p = malloc<T>();
		if(comm_.rank() == 0) new(p) T(std::forward<Args>(args)...);
		std::this_thread::sleep_for(std::chrono::seconds(rand(10)));
	//	sw_.unlock(0);
		sw_.unlock_all();
		return p;
	}
	template<class T>
	void destroy_ptr(T* ptr){
		sw_.lock_exclusive(0);
		if(comm_.rank()==0) ptr->~T();
		storage_.free_n(ptr, 1, sizeof(T));
		sw_.unlock(0);
	}
	template<class T>
	void destroy(T& t){destroy_ptr(&t);}
};

template<class T> struct allocator{
	managed_shared_memory& msm_;

	using value_type = T;
	using pointer = T*;
	using const_pointer = T const*;
	using reference = T&;
	using const_reference = T const&;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;

	allocator(managed_shared_memory& msm) : msm_(msm){}

	template<class U> 
	allocator(allocator<U> const& other) : msm_(other.msm_){}

	pointer allocate(size_type n, const void* hint = 0){
		return pointer(msm_.malloc_n<T>(n));
	}
	void deallocate(pointer ptr, size_type n){msm_.free_n<T>(ptr, n);}
	bool operator==(allocator const& other) const{
		return msm_ == other.msm_;
	}
	bool operator!=(allocator const& other) const{
		return not (other == *this);
	}
};


}}}

#ifdef _TEST_BOOST_MPI3_SHM_MANAGED_SHARED_MEMORY

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/mutex.hpp"
#include <boost/pool/pool_alloc.hpp>
#include <boost/container/string.hpp>

#include<atomic>

namespace mpi3 = boost::mpi3;
using std::cout; 

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::communicator node = world.split_shared(0);

	int N = 10;
	mpi3::shm::managed_shared_memory msm(node, 100000);

	node.barrier();
	{ // using atomics
		std::atomic<int>& i = *msm.construct<std::atomic<int>>(0);
		node.barrier();
		for(int j = 0; j != N; ++j) ++i; // std::this_thread::sleep_for(std::chrono::seconds(rand(10)));
		node.barrier();
		if(node.rank() == 0){
			int snapshot = i;
			assert( snapshot == node.size()*N );
			cout << "snapshot " << snapshot << " size " << node.size()*N << std::endl;
		}
		node.barrier();
		msm.destroy_ptr<std::atomic<int>>(&i);
	}
	node.barrier();
	{ // not using atomics
		int& i = *msm.construct<int>(0);
		node.barrier();
		for(int j = 0; j != N; ++j) ++i; // std::this_thread::sleep_for(std::chrono::seconds(rand(10)));
		node.barrier();
		if(node.rank() == 0){
			int snapshot = i;
			cout << "snapshot " << snapshot << " size " << node.size()*N << std::endl;
			assert( snapshot <= node.size()*N ); // not using atomics (or mutexes) can lead to incorrect sums
		}
		node.barrier();
		msm.destroy_ptr<int>(&i);
	}
	node.barrier();
	{
		mpi3::shm::allocator<char> ca(msm);
		using String = boost::container::basic_string<char, std::char_traits<char>, mpi3::shm::allocator<char>>; 
		String& s = *msm.construct<String>("Hello!", ca);
		if(node.rank() == 1) assert( s[1] == 'e' );
		node.barrier();
		s[node.rank()] = 'a' + node.rank();
		node.barrier();
		if(node.rank()==0) cout << s << std::endl;
		node.barrier();
		msm.destroy_ptr<String>(&s);
	}
	node.barrier();
	{
		mpi3::shm::allocator<double> da(msm);
		using Vector = boost::container::vector<double, mpi3::shm::allocator<double>>;
		Vector& v = *msm.construct<Vector>(node.size(), 1., da);
	//	v[world.rank()] = world.rank();
		world.barrier();
	//	assert( v[8] == 8. );
		msm.destroy_ptr<Vector>(&v);
	}
	node.barrier();
	return 0;
}
#endif
#endif

