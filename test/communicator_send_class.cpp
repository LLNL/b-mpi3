#if COMPILATION_INSTRUCTIONS
mpicxx -O3 -std=c++14 -Wfatal-errors -lboost_serialization $0 -o $0x.x && time mpirun -np 2 $0x.x $@ && rm -f $0x.x; exit
#endif

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

struct B{
	std::string name_ = "unnamed"; 
	int n_ = 0;
	double* data = nullptr;
	B() = default;
	B(int n) : n_(n), data(new double[n]){}
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

	std::vector<int> v(3);
	std::vector<int> s = {1,2,3};
	std::copy(v.begin(), v.end(), std::back_inserter(s));
	{
		std::vector<std::vector<double>> buffer(10, std::vector<double>(20));
		if(world.root()){
			buffer[4][5] = 6.1;
			world.send(buffer.begin(), buffer.end(), 1, 123);
		}else{
			world.receive(buffer.begin(), buffer.end(), 0, 123);
			assert( buffer[4][5] == 6.1 );
		}
	}
	{
		std::vector<double> buffer(10);
		if(world.root()){
			std::iota(buffer.begin(), buffer.end(), 0);
			world.send(buffer.begin(), buffer.end(), 1, 123);
		}else{
			std::vector<double> v(10);
		//	world.receive(std::back_inserter(v), 0, 123);
			world.receive(v.begin(), 0, 123);
		}
	}
	{
		if(world.root()){
			std::map<int, std::vector<double>> m;
			m[2] = std::vector<double>(2);
			m[5] = std::vector<double>(5);
			world.send(m.begin(), m.end(), 1, 123);
		}else{
			std::vector<std::pair<int, std::vector<double>>> v(2);
			world.receive(v.begin(), v.end(), 0, 123);
			assert(( v[1] == std::pair<int, std::vector<double>>{5, std::vector<double>(5)} ));
		}
	}
	{
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
	{
		if(world.root()){
			std::vector<B> v(5, B(3));
			v[2].data[2] = 3.14;
			world.send(v.begin(), v.end(), 1, 123);
		}else{
			std::vector<B> v(5);
			world.receive(v.begin(), v.end(), 0, 123);
			assert(v[2].data[2] == 3.14);
		}
	}
}

