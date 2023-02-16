#include <mpi3/main_environment.hpp>
namespace mpi3 = boost::mpi3;

template<class F>
float trap_local(F const f, float const local_a, float const local_b, int const local_n, float const h) {  // NOLINT(bugprone-easily-swappable-parameters)
	float integral = (f(local_a) + f(local_b)) / 2.0F;
	float x        = local_a;
	for(int i = 1; i != local_n; ++i) {  // NOLINT(altera-unroll-loops)
		integral += f(x += h);
	}

	return integral *= h;
}

template<class F>
float trap(F const f, float const a, float const b, int const n, mpi3::communicator& comm) {
	auto const my_rank = comm.rank();
	auto const p       = comm.size();

	auto const h = (b - a) / static_cast<float>(n);

	std::cout << "n = " << n << " and  p = "<< p << " n % p ="<< n % p <<std::endl;
	assert( (n % p) == 0 );
	auto const local_n = n / p;

	auto const local_a = a + static_cast<float>(my_rank * local_n) * h;
	auto const local_b = local_a + static_cast<float>(local_n) * h;

	return (comm += trap_local(f, local_a, local_b, local_n, h));
}

int mpi3::main(int /*argc*/, char** /*argv*/, mpi3::environment& env) {  // NOLINT(bugprone-exception-escape)

	auto world = env.world();

	auto const a = 0.0F;
	auto const b = 1.0F;
	int const  n = 2*3*4*6*8;

	auto const integral = trap(
		[](auto x) { return x * x; },
		/*left endpoint*/ a, /*right endpoint*/ b, /*#trapezoids*/ n, world
	);

	if(world.root()) {
		std::cout
            << "With n = " << n 
            << " trapezoids our estimate of the integral from " 
            << a << " to " << b << " is " << integral 
            << std::endl
        ;
	}

	std::cout << "integral = "<< integral << std::endl;
    assert( std::abs( integral - 1.0F/3.0F ) < 1e-5 );

	return 0;
}
