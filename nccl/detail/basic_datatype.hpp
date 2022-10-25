// -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
// Copyright 2022 Alfredo A. Correa

#ifndef MPI3_NCCL_DETAIL_BASIC_DATATYPE_HPP_
#define MPI3_NCCL_DETAIL_BASIC_DATATYPE_HPP_

#include <nccl.h>

namespace boost {
namespace mpi3 {
namespace nccl {

namespace detail {

template<class T> struct basic_datatype_t;

template<> struct basic_datatype_t<int8_t>   : std::integral_constant<ncclDataType_t, ncclInt8  > {};
template<> struct basic_datatype_t<char>     : std::integral_constant<ncclDataType_t, ncclChar  > {};
template<> struct basic_datatype_t<uint8_t>  : std::integral_constant<ncclDataType_t, ncclUint8 > {};
template<> struct basic_datatype_t<int32_t>  : std::integral_constant<ncclDataType_t, ncclInt32 > {};
	// template<> struct basic_datatype_t<int>      : std::integral_constant<ncclDataType_t, ncclInt   > {};
template<> struct basic_datatype_t<uint32_t> : std::integral_constant<ncclDataType_t, ncclUint32> {};

template<> struct basic_datatype_t<int64_t > : std::integral_constant<ncclDataType_t, ncclInt64 > {};

//template<> struct basic_datatype_t<uint64_t> : std::integral_constant<ncclDataType_t, ncclUInt64> {};

// template<> struct basic_datatype_t<float  > : std::integral_constant<ncclDataType_t, ncclFloat16 > {};
// template<> struct basic_datatype_t<float  > : std::integral_constant<ncclDataType_t, ncclHalf > {};
// template<> struct basic_datatype_t<float  > : std::integral_constant<ncclDataType_t, ncclFloat32 > {};

template<> struct basic_datatype_t<float  > : std::integral_constant<ncclDataType_t, ncclFloat  > {};

// template<> struct basic_datatype_t<float  > : std::integral_constant<ncclDataType_t, ncclFloat64 > {};

template<> struct basic_datatype_t<double > : std::integral_constant<ncclDataType_t, ncclDouble > {};

// template<> struct basic_datatype_t<float  > : std::integral_constant<ncclDataType_t, ncclBfloat64 > {};

template<class T> auto basic_datatype = basic_datatype_t<T>::value;

}

}}}

#endif
