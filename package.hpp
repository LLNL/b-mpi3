#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_PACKAGE $0x.cpp -o $0x.x && time mpirun -np 3 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_PACKAGE_HPP
#define BOOST_MPI3_PACKAGE_HPP


#ifdef _TEST_BOOST_MPI3_PACKAGE

#include "../mpi3/communicator.hpp"
#include "../mpi3/main.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	if(world.rank() == 0){
		char buf[1000];
		int i = 12;
		int j = 13;
		auto end = world.pack_n(&i, 1, buf);
		     end = world.pack_n(&j, 1, end);
		world.send_packed(buf, end, 1); //world.send_packed_n(buff, std::distance(buff, end), 1); //world.send_packed_n(buff, end - buff, 1);
		world.send_packed_n(buf, 1000, 2);
	}else if(world.rank() == 1){
		std::vector<int> v(2);
		world.receive(v.begin(), v.end(), 0);
		assert(v[0] == 12);
		assert(v[1] == 13);
	}else if(world.rank() == 2){
		char buf[1000];
		world.receive_packed_n(buf, 1000, 0);
		int i = -1;
		int j = -1;
		auto end = world.unpack_n(&i, 1, buf);
		     end = world.unpack_n(&j, 1, end);
		assert(i == 12);
		assert(j == 13);
	}
	world.barrier();
//	return 0;

	if(world.rank() == 0){
		mpi3::package p(world);
		int i = 12;
		int j = 13;
		(p << i << j).send(1).send(2);
	//	p.send(1);
	//	p.send(2);
	}else if(world.rank() == 1){
		std::vector<int> v(2, -1);
		world.receive(v.begin(), v.end(), 0);
		assert(v[0] = 12);
		assert(v[1] == 13);
	}else if(world.rank() == 2){
		mpi3::package p(world);
		int i = -1;
		int j = -1;
		p.receive(0) 
			>> i 
			>> j
		;
		assert(i == 12);
		assert(j == 13);
	}
}

#endif
#endif

