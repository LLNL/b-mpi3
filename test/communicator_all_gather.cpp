#if COMPILATION
mpicxx $0 -o $0x -lboost_serialization&&mpirun -n 3 $0x&&rm $0x;exit
#endif
// © Alfredo Correa 2018-2021

#include "../main.hpp"
#include "../communicator.hpp"
#include "../detail/iterator.hpp"

namespace mpi3 = boost::mpi3;

void part1(mpi3::communicator& world)
{
	using T = std::tuple<double, double>;
	std::vector<T> v_local(10, T{world.rank(), world.rank()});
	std::vector<T> v(v_local.size()*world.size());
	auto end = world.all_gather_n(v_local.begin(), v_local.size(), v.begin());
	assert(end == v.end());
	assert(( v[ 0] == T{0.,0.} ));
	assert(( v[10] == T{1.,1.} ));
	assert(( v[20] == T{2.,2.} ));
}

void part2(mpi3::communicator& world)
{
	using T = std::tuple<double, double>;
	std::vector<T> v_local(10, T{world.rank(), world.rank()});
	std::vector<T> v(v_local.size()*world.size());
	auto d_last = world.all_gather(begin(v_local), end(v_local), begin(v));
	assert(d_last == end(v));
	assert(( v[ 0] == T{0.,0.} ));
	assert(( v[10] == T{1.,1.} ));
	assert(( v[20] == T{2.,2.} ));
}

void part3(mpi3::communicator& world)
{
	using T = std::pair<double, int>;
	std::vector<T> v_local(10, T{world.rank(), world.rank()});
	std::vector<T> v(v_local.size()*world.size());
	auto end = world.all_gather_n(v_local.begin(), v_local.size(), v.begin());
	assert(end == v.end());
	assert(( v[ 0] == T{0.,0} ));
	assert(( v[10] == T{1.,1} ));
	assert(( v[20] == T{2.,2} ));
}

void part4(mpi3::communicator& world)
{
	using T = std::pair<double, int>;
	std::vector<T> v_local(10, T{world.rank(), world.rank()});
	std::vector<T> v(v_local.size()*world.size());
	auto d_last = world.all_gather(begin(v_local), end(v_local), begin(v));
	assert(d_last == end(v));
	assert(( v[ 0] == T{0.,0} ));
	assert(( v[10] == T{1.,1} ));
	assert(( v[20] == T{2.,2} ));
}

void part5(mpi3::communicator& world)
{
	using T = std::pair<double, int>;
	std::vector<T> v_local(world.rank() + 10, T{world.rank(), world.rank()});
	std::vector<T> v(v_local.size()*world.size());
	auto d_last = world.all_gather(begin(v_local), begin(v_local) + 4, begin(v));
	assert( std::distance(begin(v), d_last) == 4L*world.size() );
//	assert(e == end(v));
	assert(( v[ 0] == T{0.,0} ));
	assert(( v[ 4] == T{1.,1} ));
//	assert(( v[10] == T{1.,1} ));
//	assert(( v[20] == T{2.,2} ));
}

void part6(mpi3::communicator& world)
{
	auto cs = world.all_gather_as<std::vector<int> >(world.rank());
	assert(cs[0] == 0);
	assert(cs[1] == 1);
	assert(cs[2] == 2);
}

void part7(mpi3::communicator& world)
{
	using T = double;
	std::vector<T> v_local(world.rank() + 1, world.rank());
	std::vector<T> v(1 + 2 + 3);
	auto end = world.all_gatherv_n(v_local.begin(), v_local.size(), v.begin());
	assert(end == v.end());
	assert(( v[ 0] == 0. ));
	assert(( v[ 1] == 1. ));
	assert(( v[ 2] == 1. ));
	assert(( v[ 3] == 2. ));
	assert(( v[ 4] == 2. ));
	assert(( v[ 5] == 2. ));
}

auto mpi3::main(int/*argc*/, char**/*argv*/, mpi3::communicator world)->int try{
	part1(world);
	part2(world);
	part3(world);
	part4(world);
	part5(world);
	part6(world);
	part7(world);
	return 0;
}catch(...){
	return 1;
}


