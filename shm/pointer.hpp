#if COMPILATION_INSTRUCTIONS
mpic++ -D_TEST_MPI3_SHM_POINTER -xc++ $0 -o $0x&&mpirun -n 3 $0x&&rm $0x;exit
#endif
// © Alfredo A. Correa 2019-2020

#ifndef MPI3_SHM_POINTER_HPP
#define MPI3_SHM_POINTER_HPP

#include "../../mpi3/shared_communicator.hpp"
#include "../../mpi3/shared_window.hpp"

#include<boost/operators.hpp> // dereferenceable, random_access_iteratable

namespace boost{
namespace mpi3{
namespace shm{

template<class Ptr>
struct pointer_traits : std::pointer_traits<Ptr>{
	static auto to_address(Ptr const& p){
		return std::addressof(*p);
	}
};

template<class T> struct ref;

template<class T> struct pointer;

template<class T> using ptr = pointer<T>;

template<class T>
struct pointer :
	std::pointer_traits<T*>,
	std::iterator_traits<T*>,
	boost::dereferenceable<pointer<T>, ref<T>>,
	boost::random_access_iteratable<pointer<T>, T*, std::ptrdiff_t, ref<T>>
{
	template<class U> using rebind = pointer<U>;
//	using value_type = typename pointer::element_type;
//	template<class U> struct rebind{typedef pointer<U> other;};
	using std::pointer_traits<T*>::difference_type;
	using reference = ref<T>;
	std::shared_ptr<mpi3::shared_window<std::decay_t<typename pointer::element_type>>> w_;
	typename pointer::difference_type offset_;
	pointer() = default;
	pointer(std::nullptr_t) : offset_{0}{}
//	pointer(pointer const&) = default;
	pointer& operator=(pointer const& other) = default;
	template<class Other, typename = decltype(std::shared_ptr<mpi3::shared_window<typename pointer::element_type>>{std::declval<Other>().w_})> 
	pointer(Other&& o) : w_{o.w_}, offset_{o.offset_}{}
	pointer(pointer<std::remove_const_t<T>> const& o) : w_{o.w_}, offset_{o.offset_}{}
	pointer& operator=(std::nullptr_t){w_ = nullptr; offset_ = 0; return *this;}
	~pointer() = default;
//	T& operator*() const{return *(static_cast<T*>(w_->base(0)) + offset_);}
	ref<T> operator*() const{return {*this};}
	pointer& operator+=(typename pointer::difference_type d){offset_+=d; return *this;}
	pointer& operator-=(typename pointer::difference_type d){offset_-=d; return *this;}
	pointer& operator++(){++offset_; return *this;}
	pointer& operator--(){--offset_; return *this;}
	friend auto operator-(pointer const& self, pointer const& other){
		assert( self.w_ == other.w_ );
		return self.offset_ - other.offset_;
	}
//	pointer operator->() const{return wSP_->base(0) + offset_;}
//	reference operator[](difference_type d) const{return *((*this)+d);}
//	explicit operator pointer() const{return w_->base(0) + offset_;}
	friend auto raw_pointer_cast(pointer const& p){return p.w_->base(0)+p.offset_;}
	explicit operator bool() const{return bool{w_};}
	bool operator==(pointer const& o) const{assert(w_==o.w_); return offset_==o.offset_;}
	bool operator<(pointer const& o) const{assert(w_==o.w_); return offset_<o.offset_;}
	bool operator>(pointer const& o) const{assert(w_==o.w_); return offset_>o.offset_;}
	friend typename std::pointer_traits<T*>::pointer to_address(pointer const& p){
		return p.w_->base(0) + p.offset_;
	}
	template<class Size, class ForwardIt>
	auto copy_n(Size n, ForwardIt d_first) const{
		w_->fence(); 
		using std::copy_n;
		if(d_first.w_->get_group()->root()) copy_n(raw_pointer_cast(*this), n, d_first); // TODO implement with with for_each in parallel
		barrier(d_first.w_->get_group());
	}
	template<class Size, class TT>
	auto copy_n(Size n, ptr<TT> d_first) const{
		w_->fence(); using std::copy_n;
		if(d_first.w_->get_group().root()) copy_n(raw_pointer_cast(*this), n, raw_pointer_cast(d_first)); // TODO implement with with for_each in parallel
		d_first.w_->fence();
		barrier(d_first.w_->get_group());
	}
};

template<class InputIt, typename = decltype((&(*std::declval<InputIt&>())).w_->fence())>
auto copy(InputIt first){
	return [=](InputIt last, auto d_first){
		(&(*first)).w_->fence();
		if((&(*first)).w_->get_group().root())
			for( ; first != last; ++first, ++d_first) *d_first = *raw_pointer_cast(first); 
		(&(*d_first)).w_->fence();
	};
}

template<class T> 
std::true_type  is_a_ptr_(ptr<T> const&);
std::false_type is_a_ptr_(...);

template<class Ptr> struct is_a_ptr : decltype(is_a_ptr_(Ptr{})){};

template<class T> 
std::true_type  is_a_ref_(ref<T> const&);
std::false_type is_a_ref_(...);

template<class Ref> struct is_a_ref : decltype(is_a_ref_(Ref{})){};

template<class T>
class ref{
	ptr<T> pimpl_;
	ref(ptr<T> p) : pimpl_{std::move(p)}{}
	template<class TT> friend struct pointer;
public:
	ref(ref&& o) : pimpl_(o.pimpl_){} // this is needed in C++14 and below
	using decay_type = typename std::decay_t<typename ptr<T>::reference>;
	friend decltype(auto) raw_reference_cast(ref&& r){return *raw_pointer_cast(r.pimpl_);}
	ptr<T> operator&()&&{return pimpl_;}
	template<class TT>
	[[deprecated("shm slow access")]]
	auto operator=(TT&& t)&&
	->decltype(std::declval<T&>()=std::forward<TT>(t), std::declval<ref&&>()){
		pimpl_.w_->fence();
		*raw_pointer_cast(pimpl_) = std::forward<TT>(t);
		pimpl_.w_->fence();
		pimpl_.w_->fence();
		return std::move(*this);
	}
	ref&& operator=(ref&& o) &&{
		o.pimpl_.w_->fence();
		std::move(*this).operator=(static_cast<T>(*raw_pointer_cast(o.pimpl_))); 
		return std::move(*this);
	}
/*	template<class TT>
	[[deprecated("shm slow access")]]
	auto operator=(ref<TT>&& t)&&{
		std::move(*this) = 1.;
	//	t.pimpl_.w_->fence();
	//	pimpl_.w_->fence();
	//	*raw_pointer_cast(pimpl_) = std::forward<TT>(t);
	//	pimpl_.w_->fence();
	//	pimpl_.w_->fence();
		return std::move(*this);
	}*/
	[[deprecated("shm slow access")]]
	operator decay_type()&&{
		decay_type ret;
		pimpl_.w_->fence();
		pimpl_.w_->fence();
		ret = *raw_pointer_cast(pimpl_);
		return ret;
	}
	template<class TT>
	[[deprecated("shm slow access")]]
	friend auto operator==(ref&& r, TT&& tt)
	->decltype(raw_reference_cast(std::move(r))==std::forward<TT>(tt)){
		r.pimpl_.w_->fence();
		return raw_reference_cast(std::move(r))==std::forward<TT>(tt);}
};


template<class T, class F>
F for_each(pointer<T> f, pointer<T> l, F fun){ //TODO do a partitioning std::for_each
	auto& comm = f.wSP_->comm_; assert(comm == l.wSP_->comm_);
	using std::for_each;
	if(mpi3::group(*f.wSP_).root()) for_each(to_address(f), to_address(l), fun);
	f.wSP_->fence();
	f.wSP_->fence();
	return f;
}

template<class It1, typename T, typename Size>
pointer<T> copy_n(It1 f, Size n, pointer<T> d){
	d.w_->fence();
//	mpi3::communicator c(d.w_->get_group());
	if(d.w_->get_group().root()){
		using std::copy_n;
		copy_n(f, n, to_address(d));
	}
	d.w_->fence();
//	c.barrier();
//	d.wSP_->fence();
//	using std::copy_n;
//	if(mpi3::group(d.w_).root()) copy_n(f, n, to_address(d));
//	d.wSP_->fence();

	return d + n;
}

template<class It1, typename T>
pointer<T> copy(It1 f, It1 l, pointer<T> d){
	d.wSP_->fence();
	using std::copy;
	if(mpi3::group(*d.w_).root()) copy(f, l, to_address(d));
	d.wSP_->fence();
	using std::distance; using std::advance;
	advance(d, distance(f, l));
	return d;
}

/*
template<typename T, typename Size, typename TT>
pointer<T> uninitialized_fill_n(pointer<T> f, Size n, TT const& val){
	using std::uninitialized_fill_n;
	if(mpi3::group(*f.wSP_).root()) uninitialized_fill_n(to_address(f), n, val);
	f.wSP_->fence();
	f.wSP_->fence();
	return f + n;
}*/

}}}

#ifdef _TEST_MPI3_SHM_POINTER

#include "../../mpi3/main.hpp"
#include "../../mpi3/ostream.hpp"

namespace mpi3 = boost::mpi3; 

int mpi3::main(int, char*[], mpi3::communicator world){
	using p = mpi3::shm::pointer<double>;
	using cp = std::pointer_traits<p>::template rebind<double const>;//::other;
//	whatis<cp>();
	p P;
	cp CP = P;
	return 0;
}

#endif


#endif
