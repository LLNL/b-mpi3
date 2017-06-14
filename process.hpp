#ifdef COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && clang++ -O3 -std=c++14 -Wall `#-Wfatal-errors` -D_TEST_BOOST_MPI3_PROCESS -lboost_serialization $0x.cpp -o $0x.x && time mpirun -np 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_PROCESS_HPP
#define BOOST_MPI3_PROCESS_HPP

#include "../mpi3/communicator.hpp"
#include<experimental/optional>

namespace boost{
namespace mpi3{

struct process{
	communicator& comm_;
	int rank_;
	template<class T>
	optional<T> operator+=(T const& t) &&{
		T val = comm_.reduce_value(t, mpi3::sum, rank_);
		if(rank_ != comm_.rank()) return {};
		return optional<T>(val);
	}
	template<class T>
	process&& operator<<(T const& t) &&{
		comm_.send_value(t, rank_);
		return std::move(*this);
	}
	template<class T>
	process&& operator>>(T& t) &&{
		comm_.receive_value(t, rank_);
		return std::move(*this);
	}
	template<class T>
	process&& operator&(T& t) &&{
		comm_.broadcast_value(t, rank_);
		return std::move(*this);
	}
};

process communicator::operator[](int rank){
	return {*this, rank};
}

template<class T>
T operator+=(communicator& comm, T const& t){
	T val = comm.all_reduce_value(t, mpi3::sum);
	return val;
}

template<class T>
communicator& operator<<(communicator& comm, T const& t){
	comm.send_value(t);
	return comm;
}
template<class T>
communicator& operator>>(communicator& comm, T& t){
	comm.receive_value(t);
	return comm;
}
template<class T>
std::vector<T> operator|=(communicator& comm, T const& t){
	return comm.all_gather_value(t);
}

}}

#ifdef _TEST_BOOST_MPI3_PROCESS
int main(){
}
#endif
#endif


