// -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
// Copyright 2018-2023 Alfredo A. Correa

// use this to avoid linking to -lboost_serialization
// #define _MAKE_BOOST_SERIALIZATION_HEADER_ONLY

#include <mpi3/communicator.hpp>
#include <mpi3/main.hpp>

namespace mpi3 = boost::mpi3;
using std::cout;

struct A {
	std::string name_ = "unnamed";
	int         n_    = 0;
	double*     data_ = nullptr;

	A() = default;
	A(int n) : n_(n), data_{new double[n]} {}
	A(A const& other) : name_{other.name_},
	                    n_{other.n_},
	                    data_{new double[n_]} {
		std::copy_n(other.data_, n_, data_);
	}
	A& operator=(A const& other) {
		A cpy(other);
		return swap(cpy);
	}
	A& swap(A& other) {
		std::swap(name_, other.name_);
		std::swap(n_, other.n_);
		std::swap(data_, other.data_);
		return *this;
	}
	~A() { delete[] data_; }

	// begin intrusive serialization
	BOOST_SERIALIZATION_SPLIT_MEMBER()
	template<class Archive>
	void save(Archive& ar, unsigned int const) const {
		ar
			<< name_
			<< n_
			<< boost::serialization::make_array(data_, n_);
	}
	template<class Archive>
	void load(Archive& ar, unsigned int const) {
		int n;
		ar >> name_ >> n;
		;
		if(n != n_) {
			delete[] data_;
			data_ = new double[n_];
		}
		ar >> boost::serialization::make_array(data_, n_);
	}
	// end intrusive serialization
};

int mpi3::main(int, char**, mpi3::communicator world) {
	assert( world.size() == 2);

	if(world.root()) {
		std::vector<A> v(5, A(3));
		v[2].data_[2] = 3.14;
		world.send(v.begin(), v.end(), 1, 123);
	} else {
		std::vector<A> v(5);
		world.receive(v.begin(), v.end(), 0, 123);
		assert(v[2].data_[2] == 3.14);
	}
	return 0;
}
