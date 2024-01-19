// Copyright 2023-2024 Alfredo A. Correa

#include <mpi3/communicator.hpp>
#include <mpi3/environment.hpp>

namespace mpi3 = boost::mpi3;

struct spinor {
	std::complex<double> up;  // NOLINT(misc-non-private-member-variables-in-classes)
	std::complex<double> dn;  // NOLINT(misc-non-private-member-variables-in-classes)

	bool operator==(spinor const& other) const { return up == other.up and dn == other.dn; }
	bool operator!=(spinor const& other) const { return up != other.up or dn != other.dn; }
};

mpi3::environment mpienv;  // NOLINT(fuchsia-statically-constructed-objects,cert-err58-cpp,cppcoreguidelines-avoid-non-const-global-variables)

template<> struct mpi3::datatype<spinor> : mpi3::struct_<
	std::complex<double>,
	std::complex<double>
> {};

auto main(int /*argc*/, char** /*argv*/) -> int try {

	mpi3::communicator world = mpienv.world();

	using namespace std::complex_literals;  // i

	auto const I = std::complex<double>{0.0, 1.0};

	switch(world.rank()) {
	case 0: {
		std::vector<spinor> v(5);
		v[2] = spinor{3.14 + 6.28*I, 4.0 + 5.0*I};
		world.send_n(begin(v), 5, 1);
		break;
	};
	case 1: {
		std::vector<spinor> v(5);
		world.receive_n(begin(v), 5, 0);
		assert(( v[2] == spinor{3.14 + 6.28*I, 4.0 + 5.0*I} ));
		break;
	};
	}

	static_assert(boost::mpi3::has_datatype<spinor>{});
} catch(...) {
	return 1;
}
