#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 `#-Wfatal-errors` -lboost_serialization $0 -o $0x.x && time mpirun -np 2 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/communicator.hpp"
#include "alf/boost/mpi3/process.hpp"

#include "alf/boost/mpi3/detail/package_archive.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

// nontrivial nonpod class
struct B{
	std::string name_ = "unnamed"; 
	int n_ = 0;
	double* data = nullptr;
	B() = default;
	B(int n) : n_(n), data(new double[n]){
		for(int i = 0; i != n_; ++i) data[i] = 0.;
	}
	B(B const& other) : name_(other.name_), n_(other.n_), data(new double[other.n_]){}
	B& operator=(B const& other){
		name_ = other.name_;
		n_ = other.n_; 
		delete[] data; 
		data = new double[other.n_];
		for(int i = 0; i != n_; ++i) data[i] = other.data[i];
	}
	~B(){delete[] data;}
};

// nonintrusive serialization
template<class Archive>
void save(Archive & ar, B const& b, const unsigned int){
	ar << b.name_ << b.n_ << boost::serialization::make_array(b.data, b.n_);
}
template<class Archive>
void load(Archive & ar, B& b, const unsigned int){
	ar >> b.name_ >> b.n_;
	delete[] b.data; b.data = new double[b.n_];
	ar >> boost::serialization::make_array(b.data, b.n_);
}
BOOST_SERIALIZATION_SPLIT_FREE(B)

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
	assert( world.size() == 2);

	if(world.root()){
		std::vector<B> v(5, B(3));
		v[2].data[2] = 3.14;
		world.send(v.begin(), v.end(), 1, 123);
	}else{
		std::vector<B> v(5);
		world.receive(v.begin(), v.end(), 0, 123);
		assert(v[2].data[2] == 3.14);
	}

	if(world.root()){
		B b1(4); b1.data[2] = 4.5;
		world[1] << b1;
	}else{
		B b2;
		world[0] >> b2;
		assert( b2.data[2] == 4.5 );
	}

}

