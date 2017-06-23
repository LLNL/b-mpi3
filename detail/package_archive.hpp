#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 `#-Wfatal-errors` -D_TEST_BOOST_MPI3_DETAIL_PACKAGE_ARCHIVE $0x.cpp -o $0x.x -lboost_serialization && time mpirun -np 2 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif

#ifndef BOOST_MPI3_DETAIL_PACKAGE_ARCHIVE_HPP
#define BOOST_MPI3_DETAIL_PACKAGE_ARCHIVE_HPP

#include <sstream>
#include"../../mpi3/communicator.hpp"

namespace boost{
namespace mpi3{

template<class CommunicationMode, class BlockingMode, class InputIterator>
auto communicator::send_category(CommunicationMode cm, BlockingMode bm, std::input_iterator_tag, 
	InputIterator first, InputIterator last, int dest, int tag
){
	package p(world);
	detail::package_oarchive poa(p);
	for( ; first != last; ++first){
		poa << *first;
	}
	p.send(dest, tag);
}

template<class ContiguousIt, typename Size>
void communicator::broadcast_n_contiguous_builtinQ(std::false_type, ContiguousIt first, Size count, int root){
	package p(*this);
	if(rank() == root){
		detail::package_oarchive poa(p);
		while(count){
			poa << *first;
			++first;
			--count;
		}
	}
	p.broadcast(root);
	if(rank() != root){
		detail::package_iarchive pia(p);
		while(count){
			pia >> *first;
			++first;
			--count;
		}
	}
}


}}

#ifdef _TEST_BOOST_MPI3_DETAIL_PACKAGE_ARCHIVE

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/process.hpp"

#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>


namespace mpi3 = boost::mpi3;
using std::cout; 

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	if(world.rank() == 0){
		package p(world);
		int i = 12;
		int j = 13;
		mpi3::detail::package_oarchive poa(p);
		std::string s("hello");
		poa << s;
		poa << i;
		poa << j;
		std::vector<double> v(20, 5.);
		poa << v;
	//	cout << "size " << p.size() << '\n';
		cout << "size " << p.size() << '\n';
		poa << 5;
		std::map<int, int> m = {{1,2},{2,4},{3,4}};
		poa << m;
		p.send(1);
	}else if(world.rank() == 1){
		package p(world);
		p.receive(0);
		mpi3::detail::package_iarchive pia(p);
		std::string s;
		pia >> s;
		cout << "s = " << s << '\n';
		assert( s == "hello" );
		int i = 0;
		int j = 0;
		pia >> i;
		pia >> j;
		assert( i == 12 );
		assert( j == 13 );
		std::vector<double> v;
		pia >> v;
		std::cout << "v size " << v.size() << '\n';
		assert(v.size() == 20);
		int c;
		pia >> c;
		assert(c == 5);
		std::map<int, int> m;
		pia >> m;
		assert( m[3] == 4 );
	}

	if(world.rank() == 0){
		mpi3::package p(world);
		mpi3::detail::package_oarchive poa(p);
		std::vector<double> v = {12.,13};
		poa << v;
		p.send(1);
	}else if(world.rank() == 0){
		int i = 0, j = 0;
		world[0] >> i;
		world[0] >> j;
		assert( i != 12 );
		assert( j != 13 );
	}

	return 0;
}
#endif
#endif

