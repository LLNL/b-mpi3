// Copyright 2019-2024 Alfredo A. Correa

#include <mpi3/adaptors/fftw.hpp>
#include <mpi3/main_environment.hpp>

#include <complex>  // for std::norm

template<class M> auto power(M const& elem) -> decltype(std::norm(elem)) { return std::norm(elem); }

template<class M, class = std::enable_if_t<(M::rank::value >= 1)>>  // DELETE((M::rank::value < 1))>
auto power(M const& array) {
	return accumulate(begin(array), end(array), 0.0, [](auto const& alpha, auto const& omega) { return alpha + power(omega); });
}

struct sum_power {
	template<class A, class B> auto operator()(A const& alpha, B const& omega) const { return alpha + power(omega); }
};

// template<class Array>
// void chop(Array&& arr) {
// 	std::replace_if(arr.elements().begin(), arr.elements().end(), [](auto const& e) {std::fabs(e) < 1.0e-30}, 0.0);
// 	// for(auto& e : arr.elements()) {
// 	// 	if(std::fabs(e) < 1.0e-30) {
// 	// 		e = 0.0;
// 	// 	}
// 	// }
// }

namespace mpi3 = boost::mpi3;

auto mpi3::main(int /*argc*/, char** /*argv*/, boost::mpi3::environment& env) -> int try {
	mpi3::fftw::environment const fftwenv;

	auto world = env.world();

	mpi3::fftw::array<std::complex<double>, 2> G({6, 6}, 0.0, world);

	auto const [is, js] = G.local().extensions();
	std::for_each(is.begin(), is.end(), [js = js, G_local = G.local().home()](auto i) {
		std::for_each(js.begin(), js.end(), [i, &G_local](auto j) {
				G_local[i][j] = std::complex<double>{static_cast<double>(i + j), static_cast<double>(i + 2 * j)};
			}
		);
	});

	return 0;
} catch(...) {
	return 1;
}
