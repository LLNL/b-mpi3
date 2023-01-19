// -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
// Copyright 2023 Alfredo A. Correa

#include <mpi3/communicator.hpp>
#include <mpi3/main.hpp>

namespace mpi3 = boost::mpi3;

struct spinor {
	std::complex<double> up;
	std::complex<double> dn;
	bool operator==(spinor const& other) const {return up == other.up and dn == other.dn;}
    bool operator!=(spinor const& other) const {return up != other.up or  dn != other.dn;}
};

template<> struct boost::mpi3::detail::basic_datatype<spinor> : basic_datatype<
	std::pair<std::complex<double>, std::complex<double>>
> {};

auto mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator world) -> int try {
	switch(world.rank()) {
	case 0: {
		std::vector<spinor> v(5);
		v[2] = spinor{{3.14, 6.28}, {4.0, 5.0}};
		world.send(begin(v), end(v), 1);
		break;
	};
	case 1: {
		std::vector<spinor> v(5);
		world.receive(begin(v), end(v), 0);
		assert(( v[2] == spinor{{3.14, 6.28}, {4.0, 5.0}} ));
		break;
	};
	}

	return 0;
} catch(...) {
	return 1;
}
