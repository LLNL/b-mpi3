#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && mpic++ -O3 -std=c++14 -Wall `#-Wfatal-errors` -D_TEST_BOOST_MPI3_GENERALIZED_REQUEST $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_GENERALIZED_REQUEST_HPP
#define BOOST_MPI3_GENERALIZED_REQUEST_HPP

#include "../mpi3/status.hpp"
#include "../mpi3/request.hpp"


#include<mpi.h>

#include<stdexcept>

namespace boost{
namespace mpi3{

struct default_request{
	status query(){
		status ret;
		ret.set_source(MPI_UNDEFINED);
		ret.set_tag(MPI_UNDEFINED);
		ret.set_cancelled();
		ret.set_elements<char>(0);
		return ret;
	}
	void free(){}
	void cancel(int complete){}
};

struct generalized_request : request{
	template<class F>
	static int query_fn(void *extra_state, MPI_Status *status){
		try{
			*status = reinterpret_cast<F*>(extra_state)->query().impl_;
		}catch(...){
			return MPI_ERR_UNKNOWN;
		}
		return MPI_SUCCESS;
	}
	template<class F>
	static int free_fn(void* extra_state){
		try{
			reinterpret_cast<F*>(extra_state)->free();
		}catch(...){return MPI_ERR_UNKNOWN;}
		return MPI_SUCCESS;
	}
	template<class F>
	static int cancel_fn(void* extra_state, int complete){
		try{
			reinterpret_cast<F*>(extra_state)->cancel(complete);
		}catch(...){return MPI_ERR_UNKNOWN;}
		return MPI_SUCCESS;
	}
	template<class F>
	generalized_request(F& f){
		int s = MPI_Grequest_start(
			&query_fn<F>, //MPI_Grequest_query_function *query_fn,
  			&free_fn<F>, //MPI_Grequest_free_function *free_fn,
  			&cancel_fn<F>, //MPI_Grequest_cancel_function *cancel_fn,
  			std::addressof(f),	//	void *extra_state,
  			&impl_ //MPI_Request *request
		);
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot create generalized request");
	}
	void complete(){
		int s = MPI_Grequest_complete(impl_);
		if(s != MPI_SUCCESS) throw std::runtime_error("cannot complete");
	}
};

}}

#ifdef _TEST_BOOST_MPI3_GENERALIZED_REQUEST

#include "../mpi3/environment.hpp"
#include<iostream>

using std::cout;
namespace mpi3 = boost::mpi3;

struct custom_request{
	int counter = 0;
	boost::mpi3::status query(){
		counter -= 1;
		boost::mpi3::status ret;
		ret.set_source(MPI_UNDEFINED);
		ret.set_tag(MPI_UNDEFINED);
		ret.set_cancelled();
		ret.set_elements<char>(0);
		return ret;
	}
	void free(){}
	void cancel(int complete){}
};

int main(){
	mpi3::environment env;
	{
		custom_request c{};
		mpi3::generalized_request gr(c);
		assert(not gr.completed());
		gr.complete();
		gr.wait();
	}
	{
		custom_request c{1};
		mpi3::generalized_request gr(c);
		gr.complete();
		gr.wait();
		assert(c.counter == 0);
	}
}

#endif
#endif

