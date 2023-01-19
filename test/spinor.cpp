// -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
// Copyright 2023 Alfredo A. Correa

#include <mpi3/communicator.hpp>
#include <mpi3/main.hpp>

namespace mpi3 = boost::mpi3;

struct spinor {
	std::complex<double> up;  // NOLINT(misc-non-private-member-variables-in-classes)
	std::complex<double> dn;  // NOLINT(misc-non-private-member-variables-in-classes)
	bool                 operator==(spinor const& other) const { return up == other.up and dn == other.dn; }
	bool                 operator!=(spinor const& other) const { return up != other.up or dn != other.dn; }
};

template<> struct mpi3::datatype<spinor> : datatype<std::complex<double>[2]> {};  // NOLINT(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays) provide nice syntax

auto mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator world) -> int try {

    using namespace std::complex_literals;  // i

	switch(world.rank()) {
	case 0: {
		std::vector<spinor> v(5);
		v[2] = spinor{3.14 + 6.28i, 4.0 + 5.0i};
		world.send_n(begin(v), 5, 1);
		break;
	};
	case 1: {
		std::vector<spinor> v(5);
		world.receive_n(begin(v), 5, 0);
		assert(( v[2] == spinor{3.14 + 6.28i, 4.0 + 5.0i} ));
		break;
	};
	}

	static_assert(boost::mpi3::has_datatype<spinor>{});
	return 0;
} catch(...) {
	return 1;
}
