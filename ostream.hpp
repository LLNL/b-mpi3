#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_ALLOCATOR -lboost_mpi -lboost_serialization -lboost_container $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_OSTREAM_HPP
#define BOOST_MPI3_OSTREAM_HPP

#include "../mpi3/communicator.hpp"
#include "../mpi3/process.hpp"

#include <sstream>

namespace boost{
namespace mpi3{

class ostream : public std::ostream{ // http://stackoverflow.com/a/2212940/225186
	class streambuf : public std::stringbuf{
		std::ostream& output;
		communicator& comm_;
		public:
		streambuf(std::ostream& strm, communicator& comm) : output(strm), comm_(comm){}
		virtual int sync(){
			if(comm_.rank()==0){
				if(not str().empty()) output << "[" << comm_.name() << "0]" << str();
				std::string msg = "empty";
				for(int i = 1; i != comm_.size(); ++i){
					comm_[i] >> msg;
					if(not msg.empty()) output << "[" << comm_.name() << i << "]" << msg;
				}
			}else{
				comm_[0] << str();
			}
			str("");
			comm_.barrier();
			return 0;
		}
	};
	streambuf buffer;
public:
	ostream(std::ostream& os, communicator& comm) : std::ostream(&buffer), buffer(os, comm){}
	~ostream(){flush();}
};

//boost::mpi3::ostream cout(boost::mpi3::communicator const& comm){
//	return boost::mpi3::ostream(std::cout, comm);
//}

}}

#ifdef _TEST_BOOST_MPI3_ALLOCATOR
#include<iostream>

#include "alf/boost/mpi3/main.hpp"

namespace mpi3 = boost::mpi3;

int mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){

	mpi3::ostream cout(std::cout, world);
	cout << "hello, I am rank " << world.rank() << std::endl;

}


#endif
#endif

