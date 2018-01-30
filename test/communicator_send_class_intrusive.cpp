#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -lboost_serialization $0 -o $0x.x && time mpirun -np 2 $0x.x $@ && rm -f $0x.x; exit
#endif

// use this to avoid linking to -lboost_serialization
//#define MAKE_BOOST_SERIALIZATION_HEADER_ONLY

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"
#include "alf/boost/mpi3/detail/package_archive.hpp"

#include<boost/serialization/vector.hpp>
#include<boost/serialization/utility.hpp> // serialize std::pair
#include<set>

namespace mpi3 = boost::mpi3;
using std::cout;

struct A{
	std::string name_ = "unnamed"; 
	int n_ = 0;
	double* data = nullptr;
	A() = default;
	A(int n) : n_(n), data(new double[n]){}
	A(A const& other) : name_(other.name_), n_(other.n_), data(new double[other.n_]){}
	A& operator=(A const& other){
		name_ = other.name_;
		n_ = other.n_; 
		delete[] data; 
		data = new double[other.n_];
		for(int i = 0; i != n_; ++i) data[i] = other.data[i];
	}
	~A(){delete[] data;}
	// intrusive serialization
    template<class Archive>
    void save(Archive & ar, const unsigned int) const{
		ar << name_ << n_ << boost::serialization::make_array(data, n_);
    }
    template<class Archive>
    void load(Archive & ar, const unsigned int){
		ar >> name_ >> n_;
		delete[] data; data = new double[n_];
		ar >> boost::serialization::make_array(data, n_);
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
};


int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	assert( world.size() == 2);

	if(world.root()){
		std::vector<A> v(5, A(3));
		v[2].data[2] = 3.14;
		world.send(v.begin(), v.end(), 1, 123);
	}else{
		std::vector<A> v(5);
		world.receive(v.begin(), v.end(), 0, 123);
		assert(v[2].data[2] == 3.14);
	}
}

