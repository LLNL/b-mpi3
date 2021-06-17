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
	std::unique_ptr<double[]> data_;
public:
	auto data()      & -> double      *{return data_.get();}
	auto data() const& -> double const*{return data_.get();}
	auto operator[](std::size_t i)&->double &{return data()[i];}
	auto name()      & -> std::string      & {return name_;}
	auto name() const& -> std::string const& {return name_;}
	B() = default;
	explicit B(int n) : n_{n}, data_{std::make_unique<double[]>(n)}{
		std::fill_n(data_.get(), n_, 0.);
	}
	B(B const& other) : name_{other.name_}, n_{other.n_}, data_{std::make_unique<double[]>(other.n_)}{
		std::copy_n(other.data_.get(), n_, data_.get());
	}
	B(B&&) = delete;
	auto operator=(B const& other) -> B&{
		if(this == &other){return *this;}
		name_ = other.name_;
		n_ = other.n_; 
		data_ = std::make_unique<double[]>(other.n_);
		std::copy_n(other.data_.get(), n_, data_.get());
		return *this;
	}
	auto operator=(B&&) = delete;
	~B() = default;
};

// nonintrusive serialization
template<class Archive>
void save(Archive & ar, B const& b, const unsigned int/*version*/){
	ar << b.name() << b.n_ << boost::serialization::make_array(b.data_.get(), b.n_);
}
template<class Archive>
void load(Archive & ar, B& b, const unsigned int/*version*/){
	ar >> b.name() >> b.n_;
	b.data_ = std::make_unique<double[]>(b.n_);
	ar >> boost::serialization::make_array(b.data_.get(), b.n_);
}
BOOST_SERIALIZATION_SPLIT_FREE(B)

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try{
	assert( world.size() > 1 );

	switch(world.rank()){
		case 0 : {
			std::vector<B> v(5, B(3));
			v[2][2] = 3.14;
			world.send(v.begin(), v.end(), 1, 123);
		}; break;
		case 1 : {
			std::vector<B> v(5);
			world.receive(v.begin(), v.end(), 0, 123);
			assert(v[2][2] == 3.14);
		}; break;
	}
	switch(world.rank()){
		case 0 : {
			B b1(4); b1[2] = 4.5;
			world[1] << b1;
		}; break;
		case 1 : {
			B b2;
			world[0] >> b2;
			assert( b2[2] == 4.5 );
		}; break;
	}
	
	return 0;
}catch(...){throw;}

