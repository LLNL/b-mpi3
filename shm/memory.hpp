#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_SHM_MEMORY $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_SHM_MEMORY_HPP
#define BOOST_MPI3_SHM_MEMORY_HPP

#include "../../mpi3/shared_window.hpp"
namespace boost{
namespace mpi3{
namespace shm{

struct memory{
	communicator& comm_;
//	shared_window sw_;
	memory(communicator& c) : comm_(c){}
/*
	void* allocate(mpi3::size_t size){
		void* base_ptr;
		mpi3::size_t disp_unit = 1;
		shared_window ret(comm_);
		MPI_Win_allocate_shared(comm_.root()?size:0, disp_unit, MPI_INFO_NULL, comm_.impl_, &base_ptr, &ret.impl_);
		return base_ptr;
	}
	void deallocate(void* ptr){if(comm_.root()) MPI_Free_mem(ptr);}
	communicator& get_communicator(){return comm_;}
*/
};

}}}

#ifdef _TEST_BOOST_MPI3_SHM_MEMORY

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/mutex.hpp"

namespace mpi3 = boost::mpi3;
using std::cout; 

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::shared_window sw(world, 65536);
//	mpi3::shm::memory segment(world);//, 65536);
//	void* ptr = segment.allocate(100);
//	cout << "In rank " << world.rank() << " ptr is " << ptr << std::endl;
//	segment.deallocate(ptr);

}
#endif
#endif

