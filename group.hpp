#if COMPILATION_INSTRUCTIONS /*-*-indent-tabs-mode:t; c-basic-offset:4; tab-width:4-*-*/
(echo '#include"'$0'" '>$0.cpp)&&mpic++ -D_TEST_MPI3_GROUP $0.cpp -o $0x&&mpirun -n 4 $0x&&rm $0x $0.cpp;exit
#endif
// © Alfredo A. Correa 2018-2020

#ifndef MPI3_GROUP_HPP
#define MPI3_GROUP_HPP


#include "../mpi3/equality.hpp"
#include "../mpi3/error.hpp"

#include "../mpi3/detail/iterator_traits.hpp"
#include "../mpi3/detail/call.hpp"

#include<cassert>

#define OMPI_SKIP_MPICXX 1  // https://github.com/open-mpi/ompi/issues/5157
#include<mpi.h>

namespace boost{
namespace mpi3{

class group{
	MPI_Group impl_ = MPI_GROUP_NULL;
public:
	MPI_Group& operator&(){return impl_;}
	MPI_Group const& operator&() const{return impl_;}
	group() : impl_{MPI_GROUP_EMPTY}{}
	group(group&& other) noexcept : impl_{std::exchange(&other, MPI_GROUP_EMPTY)}{}
	group(group const& other){MPI_(Group_excl)(&other, 0, nullptr, &impl_);}
	void swap(group& other) noexcept{std::swap(impl_, other.impl_);}
	group& operator=(group const& other){group t{other}; swap(t); return *this;}
	group& operator=(group&& other){swap(other); other.clear(); return *this;}
	void clear(){
		if(impl_ != MPI_GROUP_EMPTY){
			MPI_(Group_free)(&impl_);
			impl_ = MPI_GROUP_EMPTY;
		}
	}
	~group(){clear();}
	group include(std::initializer_list<int> il){
		group ret; MPI_(Group_incl)(impl_, il.size(), il.begin(), &(&ret)); return ret;
	}
	group exclude(std::initializer_list<int> il){
		group ret; MPI_(Group_excl)(impl_, il.size(), il.begin(), &(&ret)); return ret;
	}
	int rank() const{int rank = -1; MPI_(Group_rank)(impl_, &rank); return rank;}
	bool root() const{assert(not empty()); return rank() == 0;}
	int size() const{int size=-1; MPI_(Group_size)(impl_, &size); return size;}
	group sliced(int first, int last, int stride = 1) const{
		int ranges[][3] = {{first, last - 1, stride}};
		group ret; MPI_(Group_range_incl)(impl_, 1, ranges, &(&ret)); return ret;
	}
	bool empty() const{return size()==0;}
	friend mpi3::equality compare(group const& self, group const& other){
		int result; 
		MPI_(Group_compare)(self.impl_, other.impl_, &result);
		return static_cast<boost::mpi3::equality>(result);
	}
	bool operator==(group const& other) const{
		mpi3::equality e = compare(*this, other); 
		return e == mpi3::identical or e == mpi3::congruent;
	}
	bool operator!=(group const& other) const{return not operator==(other);}
	bool friend is_permutation(group const& self, group const& other){
		return compare(self, other) != mpi3::unequal;
	}
	friend group set_intersection(group const& self, group const& other){
		group ret; MPI_(Group_intersection)(self.impl_, other.impl_, &(&ret)); return ret;
	}
	friend group set_difference(group const& self, group const& other){
		group ret; MPI_(Group_difference)(&self, &other, &(&ret)); return ret;
	}
	friend group set_union(group const& self, group const& other){
		group ret; MPI_(Group_union)(&self, &other, &(&ret)); return ret;
	}
	int translate_rank(int rank, group const& other) const{
		int out; MPI_(Group_translate_ranks)(impl_, 1, &rank, &other, &out); return out;
	}
};

}}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

#ifdef _TEST_MPI3_GROUP

#include "../mpi3/main.hpp"

#include<iostream>

using std::cout;
namespace mpi3 = boost::mpi3;

int mpi3::main(int, char*[], mpi3::communicator world){
	mpi3::communicator w1 = world;
	assert( w1.size() == world.size() );
	assert( w1.rank() == world.rank() );

	mpi3::group g1{w1};
	assert( g1.rank() == w1.rank() );

	mpi3::communicator w2 = w1.create(g1);
	assert( w2.size() == w1.size() );
	assert( w2.rank() == w1.rank() );
	assert( w2.rank() == world.rank() );

	return 0;
}

#endif
#endif


