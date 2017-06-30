#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_WINDOW $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_WINDOW_HPP
#define BOOST_MPI3_WINDOW_HPP

#include<mpi.h>
#include "../mpi3/communicator.hpp"
#include "../mpi3/detail/datatype.hpp"

namespace boost{
namespace mpi3{

struct window{
	public:
	MPI_Win impl_;
	window() : impl_(MPI_WIN_NULL){}
	template<class T>
	window(T* base, mpi3::size_t size, communicator& comm){
		MPI_Win_create((void*)base, size*sizeof(T), sizeof(T), MPI_INFO_NULL, comm.impl_, &impl_);
	}
	window(void* base, mpi3::size_t size, communicator& comm){
		MPI_Win_create(base, size, 1, MPI_INFO_NULL, comm.impl_, &impl_);
	}
	window(communicator& comm) : window((void*)nullptr, 0, comm){}

	window(window const&) = delete; // windows cannot be duplicated, see text before section 4.5 in Using Adv. MPI
	window(window&& other) : impl_(other.impl_){
		other.impl_ = MPI_WIN_NULL;
	}
	window& operator=(window const&) = delete;
	window& operator=(window&& other){
		if(&other == this) return *this;
		if(impl_ != MPI_WIN_NULL) MPI_Win_free(&impl_);
		impl_ = other.impl_;
		other.impl_ = MPI_WIN_NULL;
		return *this;
	}
	~window(){
		if(impl_ != MPI_WIN_NULL) MPI_Win_free(&impl_);
	}

	template<class T>
	void attach_n(T* base, mpi3::size_t n){MPI_Win_attach(impl_, base, n*sizeof(T));}

	template<typename It1, typename Size, typename V = typename std::iterator_traits<It1>::value_type>
	void accumulate_n(It1 first, Size count, int target_rank, int target_disp = 0){
		using detail::data;
		int target_count = count;
		int s = MPI_Accumulate(data(first), count, detail::basic_datatype<V>{}, target_rank, target_disp, target_count, detail::basic_datatype<V>{}, MPI_SUM, impl_); 
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot accumulate_n");
	}
//	void attach(void* base, MPI_Aint size){MPI_Win_attach(impl_, base, size);}
//	void call_errhandler(int errorcode);
	void complete() const{MPI_Win_complete(impl_);}
//	void create_errhandler(...);
//	void create_keyval(...);
//	void delete_attr(...);
	void deattach(void* base){MPI_Win_detach(impl_, base);}
	void fence(int assert_mode = 0 /*MPI_MODE_NOCHECK*/){
		MPI_Win_fence(assert_mode, impl_);
	}
//	void free_keyval(...);
	void flush(int rank){MPI_Win_flush(rank, impl_);}
	void flush_all(){MPI_Win_flush_all(impl_);}
	void flush_local(int rank){MPI_Win_flush_local(rank, impl_);}
	void flush_local_all(){MPI_Win_flush_local_all(impl_);}

	void* base() const{
		void* base; int flag;
		int i = MPI_Win_get_attr(impl_, MPI_WIN_BASE, &base, &flag);
		assert(i==0); assert(flag);
		return base;
	}
	MPI_Aint const& size() const{
		MPI_Aint* size_p; int flag;
		int i = MPI_Win_get_attr(impl_, MPI_WIN_SIZE, &size_p, &flag);
		assert(i==0); assert(flag);
		return *size_p;
	}
	int const& disp_unit() const{
		int* disp_unit_p; int flag;
		int i = MPI_Win_get_attr(impl_, MPI_WIN_DISP_UNIT, &disp_unit_p, &flag);
		assert(i==0); assert(flag);
		return *disp_unit_p;
	}

//	get_errhandler(...);
//	group get_group(){use reinterpret_cast?}
//	... get_info
//	... get_name

	void lock(int rank, int lock_type = MPI_LOCK_EXCLUSIVE, int assert = MPI_MODE_NOCHECK){
		MPI_Win_lock(lock_type, rank, assert, impl_);
	}
	void lock_exclusive(int rank, int assert = MPI_MODE_NOCHECK){
		MPI_Win_lock(MPI_LOCK_EXCLUSIVE, rank, assert, impl_);
	}
	void lock_shared(int rank, int assert = MPI_MODE_NOCHECK){
		MPI_Win_lock(MPI_LOCK_SHARED, rank, assert, impl_);
	}
	void lock_all(int assert = MPI_MODE_NOCHECK){MPI_Win_lock_all(assert, impl_);}

	void post(group const& g, int assert = MPI_MODE_NOCHECK) const{MPI_Win_post(g.impl_, assert, impl_);}
//	void set_attr(...)
//	void set_errhandler(...)
//	void set_info(...)
//	void set_name(...)
//	void shared_query(...) delegated to child class
	void start(group const& g, int assert = MPI_MODE_NOCHECK){
		MPI_Win_start(g.impl_, assert, impl_);
	}
	void sync(){MPI_Win_sync(impl_);}
//	void test(...)
	void unlock(int rank) const{MPI_Win_unlock(rank, impl_);}
	void unlock_all(){MPI_Win_unlock_all(impl_);}
	void wait() const{MPI_Win_wait(impl_);}

//	void fetch_and_op(T const*  origin, T* target, int target_rank, int target_disp = 0) const{
//		MPI_Fetch_and_op(origin, target, detail::datatype<T>{}, target_rank, target_disp, , impl_);
//	}

//	template<class T, class Op, class datatype = detail::datatype<T>, >
//	void fetch_and_op(T const*  origin, T* target, int target_rank, int target_disp = 0) const{
//		MPI_Fetch_and_op(origin, target, datatype{}, target_rank, target_disp, , impl_);
//	}

//	void fetch_exchange(T const*  origin, T* target, int target_rank, int target_disp = 0) const{
//		MPI_Fetch_and_op(origin, target,detail::datatype<T>{}, target_rank, target_disp, MPI_REPLACE, impl_);
//	}

//	maybe this goes to a pointer impl

	template<class T>
	void fetch_sum_value(T const& origin, T& target, int target_rank, int target_disp = 0) const{
		MPI_Fetch_and_op(&origin, &target, detail::basic_datatype<T>{}, target_rank, target_disp, MPI_SUM, impl_);
	}
	template<class T>
	void fetch_prod_value(T const& origin, T& target, int target_rank, int target_disp = 0) const{
		MPI_Fetch_and_op(&origin, &target, detail::basic_datatype<T>{}, target_rank, target_disp, MPI_PROD, impl_);
	}
	template<class T>
	void fetch_replace_value(T const&  origin, T& target, int target_rank, int target_disp = 0) const{
		MPI_Fetch_and_op(&origin, &target, detail::basic_datatype<T>{}, target_rank, target_disp, MPI_REPLACE, impl_);
	}
	template<class CI1, class CI2, class datatype = detail::basic_datatype<typename std::iterator_traits<CI1>::value_type> >
	void fetch_replace(CI1 it1, CI2 it2, int target_rank, int target_disp = 0) const{
		MPI_Fetch_and_op(std::addressof(*it1), std::addressof(*it2), datatype{}, target_rank, target_disp, MPI_REPLACE, impl_); 
	}

	template<class ContiguousIterator>
	void put_n(ContiguousIterator it, std::size_t n, int target_rank, int target_disp = 0) const{
		using detail::data;
		MPI_Put(
			data(it), /* void* origin_address = a + i*/ 
			n, /*int origin_count = 1 */
			detail::basic_datatype<typename std::iterator_traits<ContiguousIterator>::value_type>::value, 
			target_rank, /*int target_rank = 1*/
			target_disp, /*int target_disp = i*/
			n, /*int target_count = 1*/
			detail::basic_datatype<typename std::iterator_traits<ContiguousIterator>::value_type>::value, 
			impl_
		);
	}
	template<class Value>
	void put_value(Value const& t, int target_rank, int target_disp = 0) const{
		put_n(&t, 1, target_rank, target_disp);
	}
	template<typename ContiguousIterator, typename Size>
	void get_n(ContiguousIterator it, Size n, int target_rank, int target_disp = 0) const{
		using detail::data;
		int s = MPI_Get(
			data(it), /* void* origin_address = b + i*/
			n, /*int origin_count = 1 */
			detail::basic_datatype<typename std::iterator_traits<ContiguousIterator>::value_type>::value, 
			target_rank, /*int target_rank = 1 */
			target_disp, /*int target_disp = size1 + i*/
			n, /*int target_count = 1 */
			detail::basic_datatype<typename std::iterator_traits<ContiguousIterator>::value_type>::value, 
			impl_
		);
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot get_n");
	}
	template<class Value>
	void get_value(Value& t, int target_rank, int target_disp = 0) const{
		get_n(&t, 1, target_rank, target_disp);
	}
};

template<class T> struct reference;

template<class T>
struct shm_pointer : window{
//	T* ptr_ = nullptr;
	T* local_ptr(int rank) const{
		mpi3::size_t size;
		int disp_unit;
		void* baseptr;
		int i = MPI_Win_shared_query(window::impl_, rank, &size, &disp_unit, &baseptr);
		return static_cast<T*>(baseptr);
	}
	mpi3::size_t local_size(int rank) const{
		mpi3::size_t ret = -1;
		int disp_unit = -1;
		void* baseptr = nullptr;
		int s = MPI_Win_shared_query(window::impl_, rank, &ret, &disp_unit, &baseptr);
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot get local size");
		assert(ret%disp_unit == 0);
		return ret/disp_unit;
	}
	reference<T> operator*() const;
};

//template<class T> reference<T> pointer<T>::operator*() const{
//	return {*this};
//}

#if 0
template<class T>
shm_pointer<T> communicator::allocate_shared(MPI_Aint size) const
{
	shm_pointer<T> ret;
	// this assumes that the communicator is contained in a node
	int i = MPI_Win_allocate_shared(
		size*sizeof(T), sizeof(T), MPI_INFO_NULL, impl_, 
		&ret.ptr_, //&static_cast<window&>(ret).impl_
		&ret.window::impl_
	);
	if(i!=0) assert(0);
	return ret;
}
#endif 

template<class T>
void communicator::deallocate_shared(pointer<T> p){
//	MPI_Free_mem(p.base_ptr(rank()));
}

template<class T>
void communicator::deallocate(pointer<T>& p, MPI_Aint){
//	p.pimpl_->fence();
//	MPI_Free_mem(p.local_ptr());
//	MPI_Win_free(&p.pimpl_->impl_);
//	delete p.pimpl_;
//	p.pimpl_ == nullptr;
}


}}

#ifdef _TEST_BOOST_MPI3_WINDOW

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/allocator.hpp"
#include "alf/boost/mpi3/ostream.hpp"
#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	{
		mpi3::window w(world);
		double* darr = new double[100];
		w.attach_n(darr, 100);
		w.fence();
		delete[] darr;
	}
	return 0;
	#if 0
	{
		auto node = world.split_shared(0);
		auto p = node.allocate_shared<double>((world.rank()==0)?100:0);

		for(int i = 0; i != p.local_size(world.rank()); ++i){
			assert(world.rank() == 0);
			p.local_ptr(world.rank())[i] = 2;
		}

		assert(p.local_ptr(world.rank()) == p.ptr_);

		p.fence();

		double sum = 0;
		for(int i = 0; i != p.local_size(0); ++i) sum += p.local_ptr(0)[i];
		p.fence();
		assert(sum == 200);
	}
	#endif
	world.barrier();
	#if 0
	{
		auto p = world.malloc(world.rank()==0?100*sizeof(double):0);
		if(world.rank() == 1){
			double cinco = 5;
			p.pimpl_->lock_exclusive(0);
			p.pimpl_->put_n(&cinco, sizeof(double), 0, 11);
			p.pimpl_->unlock(0);
		}
		p.pimpl_->fence();
		if(world.rank() == 0){
			cout << p.local_ptr()[11] << endl;
			cout << p.local_size() << endl;
			cout << p.local_disp_unit() << endl;
		}
		world.deallocate(p);
	}
	#endif
	return 0;
}

#endif
#endif

