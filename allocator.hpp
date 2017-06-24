#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++17 -Wfatal-errors -D_TEST_BOOST_MPI3_ALLOCATOR -lboost_container $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_ALLOCATOR_HPP
#define BOOST_MPI3_ALLOCATOR_HPP
#include "alf/boost/version.hpp"
#include<mpi.h>
#include<numeric> // std::accumulate
#include<cassert>

#include<boost/container/allocator.hpp>

namespace boost{
namespace mpi3{

struct bad_alloc : std::bad_alloc{using std::bad_alloc::bad_alloc;};

void* malloc(std::size_t size){
	void* ret;
	int s = MPI_Alloc_mem(static_cast<MPI_Aint>(size), MPI_INFO_NULL, &ret);
	if(s != MPI_SUCCESS) throw bad_alloc();//"cannot allocate " + std::to_string(size) + " bytes");
	return ret;
}

void free(void* ptr){
	MPI_Free_mem(ptr);
}

template<class T>
struct allocator : boost::container::allocator<T>{
//	using value_type = T;
	allocator() = default;
	allocator(allocator const& other) = default;
	template<class U> allocator(allocator<U> const& other){}
	~allocator() = default;
	T* allocate(std::size_t n){return static_cast<T*>(boost::mpi3::malloc(sizeof(T)*n));}
	void deallocate(T* p, std::size_t n){boost::mpi3::free(p);}
};

template< class T1, class T2 >
bool operator==( const allocator<T1>&, const allocator<T2>& ){return true;}

template< class T1, class T2 >
bool operator!=( const allocator<T1>&, const allocator<T2>& ){return false;}

}}

#ifdef _TEST_BOOST_MPI3_ALLOCATOR

#include "alf/boost/mpi3/main.hpp"
#include<vector>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	std::vector<double, mpi3::allocator<double>> v(1000000);

	return 0;
}

#endif
#endif

