#if COMPILATION_INSTRUCTIONS
(echo "#include<"$0">" > $0x.cpp) && mpicxx -O3 -std=c++14 -Wfatal-errors -D_TEST_BOOST_MPI3_OPERATION $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_OPERATION_HPP
#define BOOST_MPI3_OPERATION_HPP

#include "../mpi3/detail/datatype.hpp"
#include "../mpi3/handle.hpp"

#include<utility> // forward

namespace boost{
namespace mpi3{

struct commutative_operation;
struct builtin_operation;

struct operation : detail::nondefault_handle<operation, MPI_Op, MPI_Op_free>{ // user_operation, operator_ . operator is a C++ keyword
	using base = detail::nondefault_handle<operation, MPI_Op, MPI_Op_free>;
	using detail::nondefault_handle<operation, MPI_Op, MPI_Op_free>::nondefault_handle;
	public:
	template<class F, typename = std::enable_if_t<not std::is_same<std::decay_t<F>, operation>{}> >
	operation(F&& f, bool commutative) : operation(detail::uninitialized{}){
		MPI_Op_create(reinterpret_cast<void (*)(void*, void*, int*, int*)>(f), commutative, &impl_);
	}

	operation() = delete;
	operation(operation const&) = delete;
	operation& operator=(operation const&) = delete;
	~operation() = default;

	enum struct code : MPI_Op{
		maximum = MPI_MAX, minimum = MPI_MIN, 
		sum = MPI_SUM, product = MPI_PROD, 
		logical_and = MPI_LAND, bitwise_and = MPI_BAND, 
		logical_or  = MPI_LOR,   bitwise_or = MPI_BOR,
		logical_xor = MPI_LXOR, bitwise_xor = MPI_BXOR,
		max_value_location = MPI_MAXLOC,
		min_value_location = MPI_MINLOC
	};
	operation(operation::code c) : base((MPI_Op)c){}

//	static operation const sum;//(operation::code::sum);
//	static operation const product;
//	static operation const maximum;
//	static operation const minimum;

};

operation const sum    (operation::code::sum);
operation const product(operation::code::product);
operation const maximum(operation::code::maximum);
operation const minimum(operation::code::minimum);

struct commutative_operation : operation{
	template<class F,  typename = std::enable_if_t<not std::is_same<std::decay_t<F>, operation>{}> >
	commutative_operation(F&& f) : operation(std::forward<F>(f), true){}
};

struct non_commutative_operation : operation{
	template<class F,  typename = std::enable_if_t<not std::is_same<std::decay_t<F>, operation>{}>>
	non_commutative_operation(F&& f) : operation(std::forward<F>(f), false){}
};

}}

#ifdef _TEST_BOOST_MPI3_OPERATION

#include "alf/boost/mpi3/main.hpp"
#include "alf/boost/mpi3/error_handler.hpp"

void addem_int(int const* invec, int *inoutvec, int *len, int* f){
	sfor(int i=0; i<*len; i++) inoutvec[i] += invec[i];
}

using std::cout;

int boost::mpi3::main(int argc, char* argv[], boost::mpi3::communicator& world){
//	boost::mpi3::operation const& op = boost::mpi3::operation(&addem_int, true);
//	boost::mpi3::operation const& op = boost::mpi3::operation::sum;
	boost::mpi3::commutative_operation op(&addem_int);
	int data = world.rank();
	int result = -1;
	world.reduce_n(&data, 1, &result, 0, op);
	world.broadcast_n(&result, 1, 0);

	int correct_result = 0;
	for(int i = 0; i != world.size(); ++i) correct_result += i;

	assert(result == correct_result);
}

#endif
#endif

