# Use cmake/ctest for building and testing boost mpi3 tests

# Not sure of the actual required minimum.
cmake_minimum_required(VERSION 3.6.0)

enable_testing()
include(CTest)

set(CMAKE_C_COMPILER mpicc)
set(CMAKE_CXX_COMPILER mpic++)

# Set BOOST_ROOT if boost is in a non-standard location
find_package(Boost REQUIRED)
if(Boost_FOUND)
  set(HAVE_LIBBOOST 1)
  include_directories(${Boost_INCLUDE_DIRS})
  message(STATUS "Setting Boost_INCLUDE_DIRS=${Boost_INCLUDE_DIRS}")
endif()

# This list is only a subset of files.  It might be useful to divide some of the tests into
#  different categories (MPI-1, MPI-2, etc., or something else)
set(TEST_SRCS
communicator_abort.cpp
communicator_all_gather.cpp
communicator_all_reduce.cpp
communicator_barrier.cpp
communicator_broadcast.cpp
communicator_divide.cpp
communicator_gather2.cpp
communicator_gather.cpp
communicator_ibroadcast.cpp
communicator_igather.cpp
communicator_ireceive.cpp
communicator_operator.cpp
communicator_reduce.cpp
communicator_scatter.cpp
communicator_send_class.cpp
communicator_send_class_nonintrusive.cpp
communicator_send.cpp
communicator_send_receive.cpp
communicator_set_error_handler.cpp
communicator_split.cpp
)

# List of tests that need to link with boost_serialization
SET(LINK_BOOST_SERIALIZATION
   communicator_send.cpp
   communicator_send_class_nonintrusive.cpp
   communicator_send_receive.cpp
   communicator_send_class.cpp
   communicator_ireceive.cpp
   communicator_gather.cpp
   process.cpp
   process_vector.cpp
)

set(CMAKE_CXX_STANDARD 14)

foreach(TEST_FILE ${TEST_SRCS})
  SET(TEST_EXE "${TEST_FILE}x.x")
  add_executable(${TEST_EXE} ${TEST_FILE})
  target_compile_options(${TEST_EXE} PUBLIC -Wall -Wextra -Wfatal-errors)
  target_include_directories(${TEST_EXE} PUBLIC "../../../..")

  list(FIND LINK_BOOST_SERIALIZATION ${TEST_FILE} NEED_BOOST_SERIALIZATION)
  if (NOT (${NEED_BOOST_SERIALIZATION} EQUAL -1))
    target_compile_options(${TEST_EXE} PUBLIC -D_MAKE_BOOST_SERIALIZATION_HEADER_ONLY)
#    target_link_libraries(${TEST_EXE} boost_serialization)
  endif()

  target_link_libraries(${TEST_EXE} pthread)

  set(NPROC 3)
  if ((TEST_FILE STREQUAL "communicator_divide.cpp") OR
      (TEST_FILE STREQUAL "communicator_operator.cpp") OR
      (TEST_FILE STREQUAL "communicator_scatter.cpp"))
    set(NPROC 8)
  endif()
  add_test(NAME ${TEST_EXE} COMMAND mpirun -n ${NPROC} ./${TEST_EXE})
endforeach()

set_tests_properties(communicator_abort.cppx.x PROPERTIES WILL_FAIL true)