#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wall -Wextra -D_MAKE_BOOST_SERIALIZATION_HEADER_ONLY `#-lboost_serialization` $0 -o $0x.x && time mpirun -n 3 $0x.x $@ && rm -f $0x.x; exit
#endif

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
#include "../../mpi3/process.hpp"


namespace mpi3 = boost::mpi3;

// nontrivial nonpod class
class B{
	std::string name_ = "unnamed";
	int n_ = 0;
	template<class Archive> friend void save(Archive & ar, B const& b, unsigned int/*version*/);
	template<class Archive> friend void load(Archive & ar, B      & b, unsigned int/*version*/);
	double* data_ = nullptr;
public:
	auto data()      & -> double      *{return data_;}
	auto data() const& -> double const*{return data_;}
	auto name()      & -> std::string      & {return name_;}
	auto name() const& -> std::string const& {return name_;}
	B() = default;
	explicit B(int n) : n_{n}, data_{new double[n]}{std::fill_n(data_, n_, 0.);}
	B(B const& other) : name_{other.name_}, n_{other.n_}, data_{new double[other.n_]}{
		std::copy_n(other.data_, n_, data_);
	}
	B(B&&) = delete;
	auto operator=(B const& other) -> B&{
		if(this == &other){return *this;}
		name_ = other.name_;
		n_ = other.n_; 
		delete[] data_; 
		data_ = new double[other.n_];
		std::copy_n(data_, n_, other.data_);
		return *this;
	}
	auto operator=(B&&) = delete;
	~B(){delete[] data_;}
};

// nonintrusive serialization
template<class Archive>
void save(Archive & ar, B const& b, const unsigned int/*version*/){
	ar << b.name() << b.n_ << boost::serialization::make_array(b.data_, b.n_);
}
template<class Archive>
void load(Archive & ar, B& b, const unsigned int/*version*/){
	ar >> b.name() >> b.n_;
	delete[] b.data_; b.data_ = new double[b.n_];
	ar >> boost::serialization::make_array(b.data_, b.n_);
}
BOOST_SERIALIZATION_SPLIT_FREE(B)

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int{
	assert( world.size() > 1 );

	switch(world.rank()){
		case 0 : {
			std::vector<B> v(5, B(3));
			v[2].data()[2] = 3.14;
			world.send(v.begin(), v.end(), 1, 123);
		}; break;
		case 1 : {
			std::vector<B> v(5);
			world.receive(v.begin(), v.end(), 0, 123);
			assert(v[2].data()[2] == 3.14);
		}; break;
	}
	switch(world.rank()){
		case 0 : {
			B b1(4); b1.data()[2] = 4.5;
			world[1] << b1;
		}; break;
		case 1 : {
			B b2;
			world[0] >> b2;
			assert( b2.data()[2] == 4.5 );
		}; break;
	}
	
	return 0;
}

