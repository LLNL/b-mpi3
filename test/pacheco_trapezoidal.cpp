#include <mpi3/main_environment.hpp>
namespace mpi3 = boost::mpi3;

template<class F>
float trap_local(F const f, float const local_a, float const local_b, int const local_n, float const h) {
	float integral = (f(local_a) + f(local_b)) / 2.0F;
	float x        = local_a;
	for(int i = 1; i != local_n; ++i) {
		integral += f(x += h);
	}

	return integral *= h;
}

template<class F>
float trap(F const f, float const a, float const b, int const n, mpi3::communicator& comm) {
	auto const my_rank = comm.rank();
	auto const p       = comm.size();

	auto const h = (b - a) / static_cast<float>(n);

	assert( n % p == 0 );
	auto const local_n = n / p;

	auto const local_a = a + static_cast<float>(my_rank * local_n) * h;
	auto const local_b = local_a + static_cast<float>(local_n) * h;

	return (comm += trap_local(f, local_a, local_b, local_n, h));
}

int mpi3::main(int /*argc*/, char** /*argv*/, mpi3::environment& env) {

	auto world = env.world();

	auto const a = 0.0F;
	auto const b = 1.0F;
	int const  n = 1024*3;

	auto const integral = trap(
		[](auto x) { return x * x; },
		/*left endpoint*/ a, /*right endpoint*/ b, /*#trapezoids*/ 1024, world
	);

	if(world.root()) {
		std::cout 
            << "With n = " << n 
            << " trapezoids our estimate of the integral from " 
            << a << " to " << b << " is " << integral 
            << std::endl
        ;
	}

    assert( std::abs( integral - 1.0F/3.0F ) < 1e-6 );

	return 0;
}
