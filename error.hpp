#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_ERROR $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_ERROR_HPP
#define BOOST_MPI3_ERROR_HPP

#include<mpi.h>

#include<string>

namespace boost{
namespace mpi3{

struct error{
	int impl_;
	error(int code){MPI_Error_class(code, &impl_);}
	int code() const{return impl_;}
	static std::string what(int code){
		int len = -1;
		char estring[MPI_MAX_ERROR_STRING];
		MPI_Error_string(code, estring, &len);
		return std::string(estring, estring + len);
	}
	std::string what() const{return what(impl_);}
	enum code {
		success = MPI_SUCCESS,
		invalid_buffer_pointer = MPI_ERR_BUFFER,
		invalid_count = MPI_ERR_COUNT,
		invalid_datatype = MPI_ERR_TYPE,
		invalid_tag = MPI_ERR_TAG,
		invalid_communicator = MPI_ERR_COMM,
		invalid_rank = MPI_ERR_RANK,
		invalid_root = MPI_ERR_ROOT,
		invalid_group = MPI_ERR_GROUP,
		invalid_operation = MPI_ERR_OP,
		invalid_topology = MPI_ERR_TOPOLOGY,
		illegal_dimension = MPI_ERR_DIMS,
		invalid_dimension = MPI_ERR_DIMS,
		invalid_argument = MPI_ERR_ARG,
		invalid_domain = MPI_ERR_ARG,
		unknown = MPI_ERR_UNKNOWN,
		truncated_message = MPI_ERR_TRUNCATE,
		other = MPI_ERR_OTHER,
		internal = MPI_ERR_INTERN,
		in_status = MPI_ERR_IN_STATUS,
		pending = MPI_ERR_PENDING,
		illegal_request = MPI_ERR_REQUEST,
		last_code = MPI_ERR_LASTCODE
	}; 
};

}}

#ifdef _TEST_BOOST_MPI3_ERROR

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/error_handler.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int boost::mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){

	cout << mpi3::error(mpi3::error::code::invalid_buffer_pointer).what() << '\n';

	return 0;
	world.set_error_handler(mpi3::error_handler::exception);
	try{
		MPI_Bcast(NULL, 0, MPI_INT, -1, MPI_COMM_WORLD);
	}catch(mpi3::exception& e){
		cout << "thrown " << e.what();
	}
	world.set_error_handler(mpi3::error_handler::code);
	int error = MPI_Bcast(NULL, 0, MPI_INT, -1, MPI_COMM_WORLD);
	cout << "error code is " << error << ", meaning " << boost::mpi3::error::what(error);
	world.set_error_handler(mpi3::error_handler::fatal);
	MPI_Bcast(NULL, 0, MPI_INT, -1, MPI_COMM_WORLD);
	assert(0);
}

#endif
#endif

