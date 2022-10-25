# Use cmake/ctest for building and testing boost mpi3 tests
cmake_minimum_required(VERSION 3.18.4) # 3.10 for FindMPI, 3.12 for MPI_ROOT
project(boostmpi3nccl_tests VERSION 0.1 LANGUAGES CXX)

enable_testing()
include(CTest)

find_package(MPI REQUIRED)

include_directories(SYSTEM ${MPI_CXX_INCLUDE_DIRS})
link_libraries(${MPI_CXX_LIBRARIES})

if (NOT MPIEXEC)
  set(MPIEXEC mpirun)
endif()


# Set BOOST_ROOT if boost is in a non-standard location
find_package(Boost REQUIRED)
if(Boost_FOUND)
  set(HAVE_LIBBOOST 1)
  include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
  message(STATUS "Setting Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")
endif()

#if(ENABLE_CUDA)
	enable_language(CUDA)
	find_package(CUDA 11.3.0 REQUIRED)
#	set(CXX_COMPILER ${CMAKE_CUDA_COMPILER})
#	set(CXX_FLAGS ${CMAKE_CUDA_FLAGS})
#	string(APPEND CMAKE_CUDA_FLAGS " --forward-unknown-to-host-linker") # ${ARCH_FLAGS} -std=c++17 --expt-relaxed-constexpr --extended-lambda --Werror=cross-execution-space-call -Xcudafe \"--diag_suppress=implicit_return_from_non_void_function\"")
#	add_compile_options(--forward-unknown-to-host-linker)
#endif()

# This list is only a subset of files.  It might be useful to divide some of the tests into
#  different categories (MPI-1, MPI-2, etc., or something else)
set(TEST_SRCS
	nccl_constructor.cpp
)

foreach(TEST_FILE ${TEST_SRCS})
  SET(TEST_EXE "${TEST_FILE}.x")
  add_executable(${TEST_EXE} ${TEST_FILE})
  target_compile_features(${TEST_EXE} PRIVATE cxx_std_17)

#  if(ENABLE_CUDA)
    set_source_files_properties(${TEST_FILE} PROPERTIES LANGUAGE CUDA)
#  endif()

  target_compile_options(${TEST_EXE} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Werror>) # -Wpedantic -Wunused -Wnon-virtual-dtor -Woverloaded-virtual -Wcast-qual -Wformat=2>) #-Wsign-conversion -Wshadow -Wconversion
  target_compile_options(${TEST_EXE} PRIVATE $<$<COMPILE_LANGUAGE:CUDA>:-Wall -Wextra>)
  target_include_directories(${TEST_EXE} PUBLIC "../../../..")

  list(FIND NEED_BOOST_SERIALIZATION_SRCS ${TEST_FILE} NEED_BOOST_SERIALIZATION)
  if (NOT (${NEED_BOOST_SERIALIZATION} EQUAL -1))
    target_compile_options(${TEST_EXE} PUBLIC -D_MAKE_BOOST_SERIALIZATION_HEADER_ONLY)
#    target_link_libraries(${TEST_EXE} boost_serialization)
  endif()

  target_link_libraries(${TEST_EXE} pthread)

  add_test(NAME ${TEST_EXE} COMMAND ${MPIEXEC} $ENV{MPI_OVERSUBSCRIBE} -n ${NPROC} ./${TEST_EXE})
  set_tests_properties(${TEST_EXE} PROPERTIES TIMEOUT 30)
endforeach()
