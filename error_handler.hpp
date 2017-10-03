#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_ERROR_HANDLER $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_ERROR_HANDLER_HPP
#define BOOST_MPI3_ERROR_HANDLER_HPP

#include<mpi.h>

//#include<stdexcept>
//#include<string>
#include "../mpi3/communicator.hpp"

namespace boost{
namespace mpi3{

//enum class error {other = MPI_ERR_OTHER};
//enum class errors : MPI_Errhandler{fatal = MPI_ERRORS_ARE_FATAL, code = MPI_ERRORS_RETURN};

class fatal{
	constexpr operator MPI_Errhandler() const{return value;}
	static constexpr MPI_Errhandler value = MPI_ERRORS_ARE_FATAL;
};
class code{
	constexpr operator MPI_Errhandler() const{return value;}
	static constexpr MPI_Errhandler value = MPI_ERRORS_RETURN;
};

struct exception : std::runtime_error{
	using std::runtime_error::runtime_error;
};

struct error_handler{
	MPI_Errhandler impl_ = MPI_ERRORS_ARE_FATAL;
	error_handler(MPI_Errhandler impl) : impl_(impl){}
	public:
	error_handler() = default;
	error_handler(void(*fn)(MPI_Comm*, int* err, ...)){
		MPI_Comm_create_errhandler(fn, &impl_);
	}
	void operator()(communicator& comm, int error) const{comm.call_error_handler(error);}
	~error_handler(){
		if(impl_ != MPI_ERRORS_ARE_FATAL and impl_ != MPI_ERRORS_RETURN)
			MPI_Errhandler_free(&impl_);
	}
	static error_handler const fatal;
	static error_handler const code;
//	static error_handler const exception;
	static void exception(MPI_Comm* comm, int* err, ...){
		int len = -1;
		char estring[MPI_MAX_ERROR_STRING];
		MPI_Error_string(*err, estring, &len);
		std::string w(estring, estring + len);
		throw boost::mpi3::exception("error code");
//		throw boost::mpi3::exception("error code " + std::to_string(*err) + " from comm " + std::to_string(*comm) + ": " + w);
//		throw std::runtime_error("error code " + std::to_string(*err) + " from comm " + std::to_string(*comm) + ": " + w);
	}

};

error_handler const error_handler::fatal(MPI_ERRORS_ARE_FATAL);
error_handler const error_handler::code(MPI_ERRORS_RETURN);
//error_handler const error_handler::exception(&exception_call);

//error_handler const error_handler::exception([](MPI_Comm* comm, int* err, ...)

void communicator::set_error_handler(error_handler const& eh){
	int status = MPI_Comm_set_errhandler(impl_, eh.impl_);
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot set error handler");
}
error_handler communicator::get_error_handler() const{
	error_handler ret;
	int status = MPI_Comm_get_errhandler(impl_, &ret.impl_);
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot get error handler");
	return ret;
}

}}

#ifdef _TEST_BOOST_MPI3_ERROR_HANDLER

#include "alf/boost/mpi3/main.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

void eh(MPI_Comm *comm, int *err, ...){
	cout << "error #" << *err << " from communicator " << comm << std::endl;
    return;
}

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	communicator comm = world;
	mpi3::error_handler newerr(eh);
	comm.set_error_handler(newerr);
	comm.call_error_handler(MPI_ERR_OTHER);
	newerr(comm, MPI_ERR_OTHER);
	auto f = comm.get_error_handler();

}

#endif
#endif

