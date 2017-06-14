#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_PORT $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_PORT_HPP
#define BOOST_MPI3_PORT_HPP

//#include "../mpi3/detail/call.hpp"
//#include
#include<mpi.h>

#include<stdexcept>
#include<string>

namespace boost{
namespace mpi3{

struct port{
	std::string name_ = ""; // typically this will be tag#0$description#inspiron$port#47425$ifname#172.17.5.240$
	port(){open();}
	port(port const&) = delete;
	port& operator=(port const& other) = delete;
	port(std::string const& name) : name_(name){};// open(name);}
	~port(){ if(is_open()) close(); }
	void open(){
		char name[MPI_MAX_PORT_NAME];
		int status = MPI_Open_port(MPI_INFO_NULL, name);
		name_ = name;
		if(status != 0) throw std::runtime_error("can't open port " + name_);
	}
	void open(std::string const& name){name_ = name;}
	std::string const& name() const{return name_;}
	bool is_open() const{return (name_ != "");}
	void close(){
		int status = MPI_Close_port(name_.c_str());
		if(status != 0) throw std::runtime_error("can't close port" + name_);
		name_ = "";
	}
};

}}

#ifdef _TEST_BOOST_MPI3_PORT

#include "alf/boost/mpi3/environment.hpp"

using std::cout;

int main(int argc, char* argv[]){
	boost::mpi3::environment env(argc, argv);
	if(env.world().size() < 3) throw std::runtime_error("Three processes needed to run this test.");
	if(env.world().rank() == 0){
		boost::mpi3::port p1;
		boost::mpi3::port p2;
		env.world().send_value(p1.name(), 1);
		env.world().send_value(p2.name(), 2);
		boost::mpi3::communicator comm1 = env.self().accept(p1, 0);
		boost::mpi3::communicator comm2 = env.self().accept(p2, 0);
		comm1.send_value(1, 0);
		comm2.send_value(2, 0);
	}else if(env.world().rank() == 1){
		boost::mpi3::port p1(env.world().receive_as<std::string>(0));
		boost::mpi3::communicator comm1 = env.self().connect(p1, 0);
		auto data = comm1.receive_as<int>(0);
		assert(data == 1);
	}else if(env.world().rank() == 2){
		boost::mpi3::port p2(env.world().receive_as<std::string>(0));
		boost::mpi3::communicator comm2 = env.self().connect(p2, 0);
		auto data = comm2.receive_as<int>(0); 
		assert(data == 2);
	}
}

#endif
#endif

