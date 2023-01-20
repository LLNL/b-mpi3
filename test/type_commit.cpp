// -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
// Copyright 2018-2023 Alfredo A. Correa

#include <mpi3/communicator.hpp>
#include <mpi3/main.hpp>
#include <mpi3/type.hpp>

#include <mpi3/detail/tuple_offset.hpp>

#include <tuple>

namespace mpi3 = boost::mpi3;

auto get_complex_2() {
	auto static ret = mpi3::type{mpi3::detail::basic_datatype<std::complex<double>>{}}[2].commit().get();
	return ret;
}

int mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator /*world*/) try {

	{
		using Tuple = std::tuple<int, double, int, char, float, long double>;
		Tuple tup;
		auto  offset4 = mpi3::detail::element_offset<4, Tuple>();
		assert(reinterpret_cast<char*>(&tup) + offset4 == reinterpret_cast<char*>(&std::get<4>(tup)));  // NOLINT(cert-dcl03-c,hicpp-static-assert,misc-static-assert,cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic) for some compiler this is not a constexpr
	}

	mpi3::type const t = mpi3::int_[100];  // mpi3::type::int_.contiguous(100);

	mpi3::type const s = mpi3::type::struct_(
		mpi3::double_complex_,
		mpi3::double_complex_
	);

	return 0;
} catch(...) {
	return 1;
}
