#if COMPILATION_INSTRUCTIONS
(echo "#include\""$0"\"" > $0x.cpp) && mpic++ -Wall -Wextra -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_ENVIRONMENT $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
//  (C) Copyright Alfredo A. Correa 2018.
#ifndef BOOST_MPI3_ENVIRONMENT_HPP
#define BOOST_MPI3_ENVIRONMENT_HPP

#include "../mpi3/communicator.hpp"

#include "../mpi3/detail/call.hpp"

#include<mpi.h>

#include<string>

namespace boost{
namespace mpi3{

struct thread_level{
//	decltype(MPI_THREAD_SINGLE)
	int impl_;
	bool operator==(thread_level const& other) const{
		return impl_ == other.impl_;
	}
};

/*[[maybe_unused]]*/ static thread_level single{MPI_THREAD_SINGLE};
/*[[maybe_unused]]*/ static thread_level funneled{MPI_THREAD_FUNNELED};
/*[[maybe_unused]]*/ static thread_level serialized{MPI_THREAD_SERIALIZED};
/*[[maybe_unused]]*/ static thread_level multiple{MPI_THREAD_MULTIPLE};

//enum struct thread_level : decltype(MPI_THREAD_SINGLE){
//	single = MPI_THREAD_SINGLE, funneled = MPI_THREAD_FUNNELED, serialized = MPI_THREAD_SERIALIZED, multiple = MPI_THREAD_MULTIPLE
//}//;

inline double wall_time(){return MPI_Wtime();}
inline double wall_tick(){return MPI_Wtick();}

struct wall_clock{
	using rep = double;
	static double time(){return wall_time();}
	static double tick(){return wall_tick();}
};
inline void finalize(){
//	MPI_Errhandler_free(&throw_error_);
	int status = MPI_Finalize();
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot finalize");
}
inline void myterminate(){
  std::cerr << "myterminate handler called" << '\n';
	finalize();
  exit(1);  // forces abnormal termination
}
inline void  initialize(int& argc, char**& argv){
	int status = MPI_Init(&argc, &argv);
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot initialize");
//	std::set_terminate(myterminate);
}
inline void throw_error_fn(MPI_Comm* comm, int* errorcode, ...){
	char name[MPI_MAX_OBJECT_NAME];
	int rlen;
	MPI_Comm_get_name(*comm, name, &rlen);
	std::string sname(name, name + rlen);
	throw std::runtime_error("error code " + std::to_string(*errorcode) + "from comm " + sname);
}
//static MPI_Errhandler throw_error_;
inline void initialize(){
	int status = MPI_Init(nullptr, nullptr);
	std::set_terminate(finalize);
//	MPI_Comm_create_errhandler(&throw_error_fn, &throw_error_);
	MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
	MPI_Comm_set_errhandler(MPI_COMM_SELF, MPI_ERRORS_RETURN);
//	MPI_Comm_set_errhandler(MPI_COMM_NULL, MPI_ERRORS_RETURN);
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot initialize");
}
inline thread_level initialize_thread(thread_level required){
	int provided;
	int status = MPI_Init_thread(nullptr, nullptr, required.impl_, &provided);
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot thread-initialize");
	return {provided};
}
inline thread_level initialize(thread_level required){
	return initialize_thread(required);
}

inline thread_level initialize_thread(int& argc, char**& argv, thread_level required){
	(void)single;
	(void)funneled;
	(void)serialized;
	(void)multiple;
	thread_level ret;
	int status = MPI_Init_thread(&argc, &argv, required.impl_, reinterpret_cast<int*>(&ret));
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot thread-initialize");
	return ret;
}
inline thread_level initialize(int& argc, char**& argv, thread_level required){
	return initialize_thread(argc, argv, required);
}
inline bool initialized(){
	int flag = -1; 
	int status = MPI_Initialized(&flag); 
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot probe initialization");
	return flag;
}
inline bool finalized(){
	int flag = -1; 
	int status = MPI_Finalized(&flag);
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot probe finalization");
	return flag;
}
inline bool is_thread_main(){
	int flag = -1;
	int status = MPI_Is_thread_main(&flag);
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot determine is thread main");
	return flag;
}
inline thread_level query_thread(){
	int ret;
	int status = MPI_Query_thread(&ret);
	if(status != MPI_SUCCESS) throw std::runtime_error("cannot query thread level");
	return {ret};
}

struct environment_base{
	 environment_base(){}//std::cout << "environment initialized\n";}
	~environment_base(){}//std::cout << "environment finalized\n";}
};

inline std::string processor_name(){return detail::call<&MPI_Get_processor_name>();}
inline std::string get_processor_name(){return detail::call<&MPI_Get_processor_name>();}

class environment : environment_base{
	public:
	environment(){initialize();}
	environment(thread_level required){initialize_thread(required);}
	environment(int argc, char** argv){initialize(argc, argv);}
	environment(int argc, char** argv, thread_level required){initialize_thread(argc, argv, required);}
	environment(environment const&) = delete;
	environment& operator=(environment const&) = delete;
	~environment(){finalize();}

	void initialize(){boost::mpi3::initialize();}
	void initialize(thread_level required){boost::mpi3::initialize_thread(required);}
	void initialize(int argc, char** argv){boost::mpi3::initialize(argc, argv);}
	void initialize(int argc, char** argv, thread_level req){boost::mpi3::initialize_thread(argc, argv, req);}

	void finalize(){boost::mpi3::finalize();}

	bool initialized() const{return boost::mpi3::initialized();}
	bool finalized() const{return boost::mpi3::finalized();}

	operator bool() const{return initialized();}
	bool is_thread_main() const{return boost::mpi3::is_thread_main();}
	thread_level query_thread() const{return boost::mpi3::query_thread();}

//	communicator& null() const{return mpi3::communicator::null;}
	communicator self() const{
		return communicator{MPI_COMM_SELF};
	}
	communicator world() const{
		communicator ret{MPI_COMM_WORLD};
		ret.name("world");
		return ret;
	}

	std::string processor_name() const{return get_processor_name();}
	std::string get_processor_name() const{return mpi3::get_processor_name();}

	double wall_time() const{return mpi3::wall_time();}
	double wall_tick() const{return mpi3::wall_tick();}

};

}}

#ifdef _TEST_BOOST_MPI3_ENVIRONMENT
#include<iostream>

namespace mpi3 = boost::mpi3;
using std::cout;

int main(int argc, char** argv){
	mpi3::environment env(argc, argv);
	[[maybe_unused]] auto world = env.world();
	return 0;
}
#endif
#endif

