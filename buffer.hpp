#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wall -Wfatal-errors -D_TEST_BOOST_MPI3_BUFFER $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_BUFFER_HPP
#define BOOST_MPI3_BUFFER_HPP

#include<mpi.h>
#include<vector>

#include<stdexcept>

namespace boost{
namespace mpi3{

template<class T>
void buffer_attach_n(T* data, std::size_t n){
	static_assert(sizeof(T)%sizeof(char) == 0, "");
	int status = MPI_Buffer_attach((void*)data, n*sizeof(T)/sizeof(char));
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot attach buffer");
}
template<class T>
void buffer_attach(T* first, T* last){
	buffer_attach_n(first, std::distance(first, last));
}
template<class C>
void buffer_attach(C& c){
	using std::begin;
	using std::end;
	return buffer_attach(begin(c), end(c));
}

template<class T, class Size>
void attach_n(T* data, Size n){return buffer_attach_n(data, n);}

template<class T>
void attach(T* first, T* last){return buffer_attach(first, last);}

template<class C>
void attach(C& c){return buffer_attach(c);}

std::pair<char*, int> buffer_detach(){
	char* buffer = 0;
	int size = -1;
	int s = MPI_Buffer_detach(&buffer, &size);
	if(s != MPI_SUCCESS) throw std::runtime_error("cannot buffer detach"); 
	return {buffer, size};
}
std::pair<char*, int> detach(){
	return buffer_detach();
}

template<class T = char>
struct scoped_attach{
	std::vector<T> buffer_;
	scoped_attach(int size){ 
		buffer_.reserve(size); 
		buffer_attach_n(buffer_.data(), size);
	}
	~scoped_attach(){
		buffer_detach();
	}
};

}}

#ifdef _TEST_BOOST_MPI3_BUFFER

#include "alf/boost/mpi3/environment.hpp"
#include<iostream>

using std::cout;
namespace mpi3 = boost::mpi3;

int main(int argc, char* argv[]){
	mpi3::environment env(argc, argv);
	std::vector<int> buffer(2000);
	mpi3::buffer_attach_n(buffer.data(), buffer.size());
	for(int j = 0; j != 10; ++j){
		env.world().buffered_send_init(
	}
	return 0;
}

#endif
#endif

