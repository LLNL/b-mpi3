#if COMPILATION_INSTRUCTIONS
mpic++ -O3 -std=c++14 -Wfatal-errors -D_MAKE_BOOST_SERIALIZATION_HEADER_ONLY `#-lboost_serialization` $0 -o $0x.x && time mpirun -n 2 $0x.x $@ && rm -f $0x.x; exit
#endif
// © Copyright Alfredo A. Correa 2018-2021

#include "../../mpi3/main.hpp"
#include "../../mpi3/communicator.hpp"
//#include "../../mpi3/detail/package_archive.hpp"

#include<boost/serialization/utility.hpp> // serialize std::pair
#include<boost/serialization/vector.hpp>

#include<set>

namespace mpi3 = boost::mpi3;

struct A{
	std::string name_ = "unnamed"; // NOLINT(misc-non-private-member-variables-in-classes) exposed for testing
	int n_ = 0;                    // NOLINT(misc-non-private-member-variables-in-classes) exposed for serialization
	std::unique_ptr<double[]> data;  // NOLINT(misc-non-private-member-variables-in-classes) exposed for serialization

	A() = default;
	explicit A(int n) : n_(n), data(new double[n]){}
	A(A const& other) : name_(other.name_), n_(other.n_), data{new double[other.n_]}{}
	A(A&&) = delete;
	auto operator=(A&&) = delete;
	auto operator=(A const& other) -> A&{
		if(this == &other){return *this;}
		name_ = other.name_;
		n_ = other.n_; 
		data.reset(new double[other.n_]); // NOLINT(cppcoreguidelines-owning-memory)
		std::copy_n(other.data.get(), n_, data.get());
		return *this;
	}
	decltype(auto) operator[](std::ptrdiff_t i){return data.get()[i];}
//	~A() noexcept{delete[] data;}
	// intrusive serialization
	template<class Archive>
	void save(Archive & ar, const unsigned int /*version*/) const{
		ar<< name_ << n_ << boost::serialization::make_array(data.get(), n_);
	}
	template<class Archive>
	void load(Archive & ar, const unsigned int /*version*/){
		ar>> name_ >> n_;
		data.reset(new double[n_]); // NOLINT(cppcoreguidelines-owning-memory)
		ar>> boost::serialization::make_array(data.get(), n_);
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()
};

struct B{
	std::string name_ = "unnamed"; // NOLINT(misc-non-private-member-variables-in-classes) exposed for serialization
	int n_ = 0;                    // NOLINT(misc-non-private-member-variables-in-classes)
	double* data = nullptr;        // NOLINT(misc-non-private-member-variables-in-classes)
	B() = default;
	explicit B(int n) : n_(n), data(new double[n]){}
	B(B const& other) : name_(other.name_), n_(other.n_), data(new double[other.n_]){}
	B(B&&) = delete;
	auto operator=(B&&) = delete;
	auto operator=(B const& other) -> B&{
		if(this == &other){return *this;}
		name_ = other.name_;
		n_ = other.n_; 
		delete[] data; 
		data = new double[other.n_];
		std::copy_n(other.data, n_, data);
		return *this;
	}
	decltype(auto) operator[](std::ptrdiff_t i){return data[i];}
	~B(){delete[] data;}
};

// nonintrusive serialization
template<class Archive>
void save(Archive & ar, B const& b, const unsigned int /*version*/){
	ar<< b.name_ << b.n_ << boost::serialization::make_array(b.data, b.n_);
}
template<class Archive>
void load(Archive & ar, B& b, const unsigned int /*version*/){ //NOLINT(google-runtime-references): serialization protocol
	ar>> b.name_ >> b.n_;
	delete[] b.data; b.data = new double[b.n_]; // NOLINT(cppcoreguidelines-owning-memory)
	ar>> boost::serialization::make_array(b.data, b.n_);
}
BOOST_SERIALIZATION_SPLIT_FREE(B)

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world) -> int try{

	assert( world.size() > 1 );

	switch(world.rank()){
		case 0 : {
			std::vector<std::vector<double>> buffer(10, std::vector<double>(20));
			buffer[4][5] = 6.1;
			world.send(buffer.begin(), buffer.end(), 1, 123);
		}; break;
		case 1 : {
			std::vector<std::vector<double>> buffer(10, std::vector<double>(20));
			world.receive(buffer.begin(), buffer.end(), 0, 123);
			assert( buffer[4][5] == 6.1 );
		}; break;
	}
	switch(world.rank()){
		case 0 : {
			std::vector<double> buffer(10);
			iota(begin(buffer), end(buffer), 0);
			world.send(begin(buffer), end(buffer), 1, 123);
		}; break;
		case 1 : {
			std::vector<double> v(10);
		//	world.receive(std::back_inserter(v), 0, 123);
			auto it = world.receive(begin(v), 0, 123);
			assert(it == v.end() and v[3] == 3.);
		}; break;
	}
	switch(world.rank()){
		case 0: {
			std::map<int, std::vector<double>> m;
			m[2] = std::vector<double>(2);
			m[5] = std::vector<double>(5);
			world.send(begin(m), end(m), 1, 123);
		}; break;
		case 1: {
			std::vector<std::pair<int, std::vector<double>>> v(2);
			world.receive(begin(v), end(v), 0, 123);
			assert(( v[1] == std::pair<int, std::vector<double>>{5, std::vector<double>(5)} ));
		}; break;
	}
	switch(world.rank()){
		case 0 : {
			std::vector<A> v(5, A(3));
			v[2][2] = 3.14;
			world.send(begin(v), end(v), 1, 123);
		}; break;
		case 1 : {
			std::vector<A> v(5);
			world.receive(begin(v), end(v), 0, 123);
			assert(v[2][2] == 3.14);
		}; break;
	}
	switch(world.rank()){
		case 0 : {
			std::vector<B> v(5, B(3));
			v[2][2] = 3.14;
			world.send(begin(v), end(v), 1, 123);
		}; break;
		case 1 : {
			std::vector<B> v(5);
			world.receive(begin(v), end(v), 0, 123);
			assert(v[2][2] == 3.14);
		}; break;
	}

	return 0;
}catch(...){
	return 1;
}

