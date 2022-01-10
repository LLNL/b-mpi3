#pragma once
#include<tuple>

namespace boost::mpi3::detail{

template <std::size_t I, typename Tuple>
constexpr std::size_t tuple_offset_aux() {
    static_assert(!std::is_reference_v<std::tuple_element_t<I, Tuple>>);
    union u {
        constexpr u() : a{} {}  // GCC bug needs a constructor definition
        char a[sizeof(Tuple)];  // NOLINT(cppcoreguidelines-avoid-c-arrays,hicpp-avoid-c-arrays,modernize-avoid-c-arrays)
        Tuple t;
    } x;
    std::size_t off = 0;
    while(static_cast<void*>(x.a + off) != std::addressof(std::get<I>(x.t))) {++off;}  // NOLINT(cppcoreguidelines-pro-type-union-access,cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return off;
}

template<std::size_t I, typename Tuple>
struct tuple_offset : std::integral_constant<std::size_t, tuple_offset_aux<I, Tuple>()> {};

template<std::size_t I, typename Tuple>
constexpr std::size_t tuple_offset_v = tuple_offset<I, Tuple>::value;

}  // end namespace boost::mpi3::detail
