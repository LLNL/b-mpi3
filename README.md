<!--- &2>/dev/null
    xdg-open $0; exit
--->
[comment]: # (Comment)

# B.MPI3
*Alfredo A. Correa*
<correaa@llnl.gov>

[//]: <> (<alfredo.correa@gmail.com>)

B-MPI3 is a C++ library wrapper for version 3.1 of the MPI standard interface that simplifies the utilization and maintenance of MPI code.
B-MPI3 C++ aims to provide a more convenient, powerful and an interface less prone to errors than the standard C-based MPI interface.

B-MPI3 simplifies the utilization of MPI without completely changing the communication model, allowing for a seamless transition from C-MPI.
B-MPI3 also provides allocators and facilities to manipulate MPI-mediated Remote Access and shared memory.

For example, pointers are not utilized directly and it is replaced by an iterator-based interface and most data, in particular custom type objects are serialized automatically into messages by the library.
B-MPI3 interacts well with the C++ standard library, containers and custom data types (classes).

B.MPI3 is written from scratch in C++14 and it has been tested with many MPI library implementations and compilers, OpenMPI +1.9, MPICH +3.2.1, MVAPICH or Spectrum MPI, using the following compilers gcc +5.4.1, clang +6.0, PGI 18.04.
(Any standard compliant MPI library can be used.)

B.MPI3 is not an official Boost library, but is designed following the principles of Boost and the STL.
B.MPI3 is not a derivative of Boost.MPI and it is unrelated to the, now deprecated, official MPI-C++ interface.
It adds features which were missing in Boost.MPI (which only covers MPI-1), with an iterator-based interface and MPI-3 features (RMA and Shared memory).

B.MPI3 optionally depends on Boost +1.53 for automatic serialization.

## Introduction

MPI is a large library for run-time parallelism where several paradigms coexist.
It was is originally designed as standardized and portable message-passing system to work on a wide variety of parallel computing architectures.

The last standard, MPI-3, uses a combination of techniques to achieve parallelism, Message Passing (MP), (Remote Memory Access (RMA) and Shared Memory (SM).
We try here to give a uniform interface and abstractions for these features by means of wrapper function calls and concepts brought familiar to C++ and the STL.

## Motivation: The problem with the standard interface

A typical C-call for MP looks like this,

```cpp
int status_send = MPI_Send(&numbers, 10, MPI_INT, 1, 0, MPI_COMM_WORLD);
assert(status_send == MPI_SUCCESS);
... // concurrently with 
int status_recv = MPI_Recv(&numbers, 10, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
assert(status_recv == MPI_SUCCESS);
```

In principle this call can be made from a C++ program.
However there are obvious drawbacks from using this standard interface.

Here we enumerate some of problems,

* Function calls have many arguments (e.g. 6 or 7 arguments in average)
* Many mandatory arguments are redundant or could easily have a default natural value (e.g. message tags are not always necessary).
* Use of raw pointers and sizes, (e.g. `&number` and `1`)
* Data argument are type-erased into `void*`.
* Only primitive types (e.g. `MPI_INT`) can be passed.
* Consistency between pointer types and data-types is responsibility of the user.
* Only contiguous memory blocks can be used with this interface.
* Error codes are stored and had to be checked after each function call.
* Use of handles (such as `MPI_COMM_WORLD`), handles do not have a well defined semantics.

A call of this type would be an improvement:

```cpp
world.send(numbers.begin(), numbers.end(), 1);
... // concurrently with 
world.receive(numbers.begin(), numbers.end(), 0); 
```

For other examples, see here: [http://mpitutorial.com/tutorials/mpi-send-and-receive/](http://mpitutorial.com/tutorials/mpi-send-and-receive/)

MPI used to ship with a C++-style interfaces.
It turns out that this interface was a very minimal change over the C version, and for good reasons it was dropped.

The B.MPI3 library was designed to use simultaneously (interleaved) with the standard C interface of MPI.
In this way, changes to existing code can be made incrementally.

## Installation

The library is "header-only"; no separate compilation is necessary.
Most functions are inline or template functions.
In order to compile it requires an MPI distribution (e.g. OpenMPI or MPICH2) and the corresponding compiler-wrapper (`mpic++` or `mpicxx`).
Currently the library requieres C++14 (usually activated with the compiler option `-std=c++14`) and Boost. In particular it depends on Boost.Serialization and may require linking to this library if values passed are not basic types (`-lboost_serialization`). A typical compilation/run command looks like this:

```bash
$ mpic++ -std=c++14 -O3 mpi3/test/communicator_send.cpp -o communicator_send.x -lboost_serialization
$ mpirun -n 8 ./communicator_send.x
```

In a system such as Red Hat, the dependencies can by installed by

```bash
$ dnf install gcc-c++ boost-devel openmpi-devel mpich-devel
```

The library is tested frequently against `openmpi` and `mpich`, and less frequently with `mvapich2`.

## Testing

The library has a basic `ctest` based testing system.

```bash
cd mpi3/test
mkdir build; cd build
cmake .. && make && ctest
```

## Initialization

Like MPI, B.MPI3 requires some global library initialization.
The library includes a convenience `mpi3/main.hpp` which wraps around this initialization steps and *simulates* a main function. 
In this way, a parallel program looks very much like normal programs, except that the main function has a third argument with the default global communicator passed in.

```cpp
#include "mpi3/version.hpp"
#include "mpi3/main.hpp"

#include<iostream>

namespace mpi3 = boost::mpi3; 
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator world){
	if(world.rank() == 0) cout << mpi3::version() << '\n';
	return 0;
}
```

Here `world` is a communicator object that is a wrapper over MPI communicator handle.

Changing the `main` program to this syntax in existing code can be too intrusive. 
For this reason a more traditional initialization is also possible.
The alternative initialization is done by instantiating the `mpi3::environment` object (from with the global communicator `.world()` is extracted).

```cpp
#include "mpi3/environment.hpp"
int main(int argc, char** argv){
	mpi3::environment env(argc, argv);
	auto world = env.world(); // communicator is extracted from the environment 
    // ... code here
	return 0;
}
```

## Communicators

In the last example, `world` is a global communicator (not necessarely the same as `MPI_COMM_WORLD`, but a copy of it).
There is no global communicator variable `world` that can be accessed directly in a nested function.
The idea behind this is to avoid using the global communicators in nested functions of the program unless they are explicitly passed in the function call.
Communicators are usually passed by reference to nested functions.
Even in traditional MPI it is a mistake to assume that the `MPI_COMM_WORLD` is the only available communicator.

`mpi3::communicator` represent communicators with value-semantics.
This means that `mpi3::communicator` can be copied or passed by reference.
A communicator and their copies are different entities that compare equal.
Communicators can be empty, in a state that is analogous to `MPI_COMM_NULL` but with proper value semantics.

Like in MPI communicators can be duplicated (copied into a new instance) or split.
They can be also compared. 

```cpp
mpi3::communicator world2 = world;
assert( world2 == world );
mpi3::communicator hemisphere = world/2;
mpi3::communicator interleaved = world%2;
```

This program for example splits the global communicator in two sub-communicators one of size 2 (including process 0 and 1) and one with size 6 (including 2, 3, ... 7);

```cpp
#include "mpi3/main.hpp"
#include "mpi3/communicator.hpp"

namespace mpi3 = boost::mpi3;
using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator world){
    assert(world.size() == 8); // this program can only be run in 8 processes
    mpi3::communicator comm = (world <= 1);
    assert(!comm || (comm && comm.size() == 2));
    return 0;
}
```

Communicators give also index access to individual `mpi3::processes` ranging from `0` to `comm.size()`. 
For example, `world[0]` referrers to process 0 or the global communicator.
An `mpi3::process` is simply a rank inside a communicator.
This concept doesn't exist explicit in the standard C interface, but it simplifies the syntax for message passing.

Splitting communicators can be done more traditionally via the `communicator::split` member function. 

Communicators are used to pass messages and to create memory windows.
A special type of communicator is a shared-communicator `mpi3::shared_communicator`.

## Message Passing

This section describes the features related to the message passing (MP) functions in the MPI library.
In C-MPI information is passed via pointers to memory.
This is expected in a C-based interface and it is also very efficient.
In Boost.MPI, information is passed exclusively by value semantics. 
Although there are optimizations that amortize the cost, we decided to generalize the pointer interface and leave the value-based message passing for a higher-level syntax. 

Here we replicate the design of STL to process information, that is, aggregated data is passed mainly via iterators. (Pointer is a type of iterator).

For example in STL data is copied between ranges in this way.
```cpp
std::copy(origin.begin(), origin.end(), destination.begin());
```

The caller of function copy doesn't need to worry about he type of the `origin` and `destination` containers, it can mix pointers and iterators and the function doesn't need more redundant information than the information passed. 
The programmer is responsible for managing the memory and making sure that design is such that the algorithm can access the data referred by the passed iterators.

Contiguous iterators (to built-in types) are particularity efficient because they can be mapped to pointers at compile time. This in turn is translated into a MPI primitive function call.
The interface for other type of iterators or contiguous iterators to non-build-in type are simulated, mainly via buffers and serialization.
The idea behind this is that generic message passing function calls can be made to work with arbitrary data types.

The main interface for message passing in B.MPI3 are member functions of the communicator.
For example `communicator::send`, `::receive` and `::barrier`. 
The functions `::rank` and `::size` allows each process to determine their unique identity inside the communicator.

```cpp
int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
    assert(world.size() == 2);
	if(world.rank() == 0){
	   std::vector<double> v = {1.,2.,3.};
	   world.send(v.begin(), v.end(), 1); // send to rank 1
	}else if(world.rank() == 1){
	   std::vector<double> v(3);
	   world.receive(v.begin(), v.end(), 0); // receive from rank 1
	   assert( v == std::vector{1.,2.,3.} );
	}
	world.barrier(); // synchronize execution here
	return 0;
}
```

Other important functions are `::gather`, `::broadcast` and `::accumulate`. 
This syntax has a more or less obvious (but simplified) mapping to the standard C-MPI interface.
In Boost.MPI3 however all, these functions have reasonable defaults that make the function call shorted and less prone to errors and with the C-MPI interface.

For more examples, look into `./mpi3/tests/`, `./mpi3/examples/` and `./mpi3/exercises/`.

The interface described above is iterator based and is a direct generalization of the C-interface which works with pointers.
If the iterators are contiguous and the associated value types are primitive MPI types, the function is directly mapped to the C-MPI call.

Alternatively, value-based interface can be used.
We will show the terse syntax, using the process objects.

```cpp
int mpi3::main(int argc, char* argv[], mpi3::communicator& world){
    assert(world.size() == 2);
	if(world.rank() == 0){
	   double v = 5.;
	   world[1] << v;
	}else if(world.rank() == 1){
	   double v = -1.;
	   world[0] >> v;
	   assert(v == 5.);
	}
	return 0;
}
```

## Remote Memory Access

Remote Memory (RM) is handled by `mpi3::window` objects. 
`mpi3::window`s are created by `mpi3::communicator` via a collective (member) functions.
Since `mpi3::window`s represent memory, it cannot be copied (but can be moved). 

```cpp
mpi3::window w = world.make_window(begin, end);
```

Just like in the MPI interface, local access and remote access is synchronized by a `window::fence` call.
Read and write remote access is performed via put and get functions.

```cpp
w.fence();
w.put(begin, end, rank);
w.fence();
```

This is minimal example using `put` and `get` functions.

```cpp
#include "mpi3/main.hpp"
#include<iostream>

namespace mpi3 = boost::mpi3; using std::cout;

int mpi3::main(int, char*[], mpi3::communicator world){

	std::vector<double> darr(world.rank()?0:100);
	mpi3::window<double> w = world.make_window(darr.data(), darr.size());
	w.fence();
	if(world.rank() == 0){
		std::vector<double> a = {5., 6.};
		w.put(a.begin(), a.end(), 0);
	}
	world.barrier();
	w.fence();
	std::vector<double> b(2);
	w.get(b.begin(), b.end(), 0);
	w.fence();
	assert( b[0] == 5.);
	world.barrier();

	return 0;
}
```

In this example, memory from process 0 is shared across the communicator, and accessible through a common window.
Process 0 writes (`window::put`s) values in the memory (this can be done locally or remotely). 
Later all processes read from this memory. 
`put` and `get` functions take at least 3 arguments (and at most 4).
The first two is a range of iterators, while the third is the destination/source process rank (called "target_rank"). 

Relevant examples and test are located in For more examples, look into `./mpi3/tests/`, `./mpi3/examples/` and `./mpi3/exercises/`.

`mpi3::window`s may carry type information (as `mpi3::window<double>`) or not (`mpi3::window<>`)

## Shared Memory

Shared memory (SM) uses the underlying capability of the operating system to share memory from process within the same node. 
Historically shared memory has an interface similar to that of remove access.
Only communicators that comprise a single node can be used to create a share memory window.
A special type of communicator can be created by splitting a given communicator.

`mpi3::shared_communicator node = world.split_shared();`

If the job is launched in single node, `node` will be equal (congruent) to `world`.
Otherwise the global communicator will be split into a number of (shared) communicators equal to the number of nodes.

`mpi3::shared_communicator`s can create `mpi3::shared_window`s. 
These are special type of memory windows.

```cpp
#include "mpi3/main.hpp"

namespace mpi3 = boost::mpi3; using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::shared_communicator node = world.split_shared();
	mpi3::shared_window<int> win = node.make_shared_window<int>(node.rank()==0?1:0);

	assert(win.base() != nullptr and win.size<int>() == 1);

	win.lock_all();
	if(node.rank()==0) *win.base<int>(0) = 42;
	for (int j=1; j != node.size(); ++j){
		if(node.rank()==0) node.send_n((int*)nullptr, 0, j);//, 666);
	    else if(node.rank()==j) node.receive_n((int*)nullptr, 0, 0);//, 666);
	}
	win.sync();

	int l = *win.base<int>(0);
	win.unlock_all();

	int minmax[2] = {-l,l};
	node.all_reduce_n(&minmax[0], 2, mpi3::max<>{});
	assert( -minmax[0] == minmax[1] );
	cout << "proc " << node.rank() << " " << l << std::endl;

	return 0;
}
```

For more examples, look into `./mpi3/tests/`, `./mpi3/examples/` and `./mpi3/exercises/`.

# Beyond MP, RMA and SHM

MPI provides a very low level abstraction to inter-process communication.
Higher level of abstractions can be constructed on top of MPI and by using the wrapper the works is simplified considerably.

## Mutex

Mutexes can be implemented fairly simply on top of RMA.
Mutexes are used similarly than in threaded code, 
it prevents certain blocks of code to be executed by more than one process (rank) at a time.

```cpp
#include "mpi3/main.hpp"
#include "mpi3/mutex.hpp"

#include<iostream>

namespace mpi3 = boost::mpi3; using std::cout;

int mpi3::main(int argc, char* argv[], mpi3::communicator& world){

	mpi3::mutex m(world);
	{
		m.lock();
		cout << "locked from " << world.rank() << '\n';
		cout << "never interleaved " << world.rank() << '\n';
		cout << "forever blocked " << world.rank() << '\n';
		cout << std::endl;
		m.unlock();
	}
	return 0;
}
```

(Recursive mutexes are not implemented yet)

Mutexes themselves can be used to implement atomic operations on data.

# Ongoing work

We are implementing memory allocators for remote memory, atomic classes and asynchronous remote function calls.
Higher abstractions and use patterns will be implemented, specially those that fit into the patterns of the STL algorithms and containers.

# Advanced Topics

## Thread safety

If you are not using threads at all, you can skip this section, however if you read it you may understand some design decisions taken by the library.

Thread-safety with MPI is extremely complicated, as there are various aspects to it, from the data communicated, to the communicator itself, to the runtime system.
This library doesn't try to hide this fact; in place, it provides the tools available to C++ to deal with this complication.
As we will see, there are certain steps to take to make the code _compatible_ with threads.

Absolute thread-safety is a very strong guarantee and it would come at a very steep performance cost.
Almost no general purpose library guarantees complete thread safety.
In opposition to thread-safety, we will discuss thread-compatibility which is a more reasonable goal.
Thread-compatibility referres to the property of a system to be able to be thread-safe if extra steps are taken and only when needed.

The first condition for thread compatibility is to have an MPI environment that supports it.
If you have an MPI system provides only a `thread_support` at the level of `mpi3::thread::single` it means that there is no way to make MPI operations from different threads an expect correct results.
If your program expect to call MPI in concurrent portition sections, your only option would be to change to a system that supports MPI threading.

In this small example, we assume that the program excpects threading and MPI and completely rejects the run if the any level different from `single` is not provided. 
This is not at all terrible choice, optionally supporting threading in a big program can be prohibitive from a design point of view.

```cpp
int main() {
	mpi3::environment env{mpi3::thread::multiple};
	switch( env.thread_support() ) {
		case mpi3::thread::single    : throw std::logic_error{"threads not supported"};
		case mpi3::thread::funneled  : std::cout<<"funneled"  <<std::endl;
		case mpi3::thread::serialized: std::cout<<"serialized"<<std::endl;
		case mpi3::thread::multiple  : std::cout<<"multiple"  <<std::endl;
	}
	...
```

Alternatively you can just check that `env.thread_suppost() > mpi3::single`, since the levels `multiple > serialized > funneled > single` are ordered.

### Data

Even if MPI operations are called outside concurrent sections it is still be your responsibility to make sure that the *data* involved is synchronized, this is always the case.
Clear ownership and scoping of *data* helps a lot toward the thread safety. 
Avoiding mutable shared data between threads also helps a lot. 
As a last resort, data can be locked with std::mutex`-like object to be written or accessed one thread at time.

### Communicator

The library doesn't control or owns data for the most part, therefore the main concern regarding threading that the library is within the communicator class itself.

The C-MPI interface briefly mentions thread-safety, for example most MPI operations are acompained by the following note (e.g. https://www.mpich.org/static/docs/latest/www3/MPI_Send.html):

> ### Thread and Interrupt Safety
> 
> This routine is thread-safe. This means that this routine may be safely used by multiple threads without the need for any  user-provided thread locks. However, the routine is not interrupt safe. Typically, this is due to the use of memory allocation routines such as malloc or other non-MPICH runtime routines that are themselves not interrupt-safe. 

This doesn't mean that that *all* calls to, for example, `MPI_Send`, can be safely done from different threads concurrently, only some of them, a.k.a. with completely different argument can be safe.

In practice it is observable that for most MPI operations the "state" of the communicator can change in time.
Even if after the operation the communicator seems to be in the same state as before the call the operation itself changes briefly the state of the object.
This internal state can be observed from another thread even through undefined behavior.
In modern C++, this is enough to mark communicator operations a no-`const` (i.e. an operation than can be applied only on a mutable communicator).

`MPI_Send` has "tags" to differenciate separate communicators, this is still not a enough since the tag is a runtime variable.
Agreeing in tags would in itself require synchronization.
Besides most collective operation do not have tags at all. 
It has been known for a while that the identity of the communicator in some sense serves as a tag for collective communications.
This is why it is so useful to be able to duplicate communicators.

This brings us to the important topic of communicator construction and assigment.


# Conclusion

The goal is to provide a type-safe, efficient, generic interface for MPI.
We achieve this by leveraging template code and classes that C++ provides.
Typical low-level use patterns become extremely simple, and that exposes higher-level patterns.

# Mini tutorial

This section describes the process of bringing a C++ program that uses the original MPI interface to one that uses B.MPI3.
Below it is a valid C++ MPI program using send and receive function.
Due to the legacy nature of MPI, C and C++ idioms are mixed.

```cpp
#include<mpi.h>

#include<iostream>
#include<numeric>
#include<vector>

int main(int argc, char **argv) {
	MPI_Init(&argc, &argv);
	MPI_Comm comm = MPI_COMM_WORLD;

	int count = 10;

	std::vector<double> xsend(count); iota(begin(xsend), end(xsend), 0);
	std::vector<double> xrecv(count, -1);

	int rank = -1;
	int nprocs = -1;
	MPI_Comm_rank(comm, &rank);
	MPI_Comm_size(comm, &nprocs);
	if(nprocs%2 == 1) {
	   if(rank == 0) {std::cerr<<"Must be called with an even number of processes"<<std::endl;}
	   return 1;
	}

	int partner_rank = (rank/2)*2 + (rank+1)%2;

	MPI_Send(xsend.data(), count, MPI_DOUBLE, partner_rank  , 0          , comm);
	MPI_Recv(xrecv.data(), count, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, comm, MPI_STATUS_IGNORE);
	assert(xrecv[5] == 5);

	if(rank == 0) {std::cerr<<"successfully completed"<<std::endl;}

	MPI_Finalize();
	return 0;
}
```

We are going to work "inward", with the idea of mimicking the process of modernizing a code from the top (the opposite it is also feasible).
This process is typical if the low level code needs to stay untouched for historical reasons.

The first step is to include the wrapper library and, as a warm up, replace the `Init`, `Finalize` calls.
At the same time we obtain the (global) world communicator from the library.


```cpp
#include "../../mpi3/environment.hpp"

#include<iostream>
#include<numeric>
#include<vector>

namespace bmpi3 = boost::mpi3;

int main(int argc, char **argv) try {
	bmpi3::environment::initialize(argc, argv);
	MPI_Comm comm = &bmpi3::environment::get_world_instance(); assert(comm == MPI_COMM_WORLD)
...
	bmpi3::environment::finalize();
	return 0;
}
```

Notice that we are getting a reference to the global communicator using the `get_world_instance`, then, with the ampersand (`&`) operator, we obtain a `MPI_Comm` handle than can be used with the rest of the code untouched.

Since `finalize` will need to be exectuted in any path, it is preferrable to use an RAII object to represent the environment.
Just like in classic MPI, it is wrong to create more than one environment.

Both, accesing the global communicator directky is in general considered problematic.
For this reason it makes more sense to ask for a duplicate of the global communicator.

```cpp
int main(int argc, char **argv) {
	bmpi3::environment env(argc, argv);
	bmpi3::communicator world = env.world();
	MPI_Comm comm = &world; assert(comm != MPI_COMM_WORLD);
...
	return 0;
}
```

This ensures that `finalize` is always called (by the destructor) and that we are not using the original global communicator, but a duplicate.

Since this pattern is very common, a convenient "main" function is declared by the library as a replacement declared in the `mpi3/main.hpp` header.

```cpp
#include "../../mpi3/main.hpp"

#include<iostream>
#include<numeric>
#include<vector>

namespace bmpi3 = boost::mpi3;

int bmpi3::main(int, char **, bmpi3::communicator world) {
	MPI_Comm comm = &world; assert(comm != MPI_COMM_WORLD);
...
	return 0;
}
```

The next step is to replace the use of the MPI communicator handle by a proper `mpi3::communicator` object.
Since `world` is already a duplicate of the communicator we can directly use it.
The `size` and `rank` are methods of this object which naturally return their values.

```cpp
...
	int rank = world.rank();
	int nprocs = world.size();
...
```

Similarly the calls to send and receive data can be transformed.
Notice that the all the irrelevant or redundant arguments (including the receive source) can be omitted.

```cpp
...
	world.send_n   (xsend.data(), count, partner_rank);
	world.receive_n(xrecv.data(), count);
...
```

(We use the `_n` suffix interface to emphasize that we are using element count (container size) as argument.)

The condition `(rank == 0)` is so common that can be replaced by the `communicator`'s method `is_root()`:

```cpp
	if(world.is_root()) {std::cerr<<"Must be called with an even number of processes"<<std::endl;}
```

```cpp
#include "../../mpi3/main.hpp"

#include<iostream>
#include<numeric>
#include<vector>

namespace bmpi3 = boost::mpi3;

int bmpi3::main(int /*argc*/, char ** /*argv*/, bmpi3::communicator world) try {
	int count = 10;

	std::vector<double> xsend(count); iota(begin(xsend), end(xsend), 0);
	std::vector<double> xrecv(count, -1);

	if(world.size()%2 == 1) {
	   if(world.is_root()) {std::cerr<<"Must be called with an even number of processes"<<std::endl;}
	   return 1;
	}

	int partner_rank = (world.rank()/2)*2 + (world.rank()+1)%2;

	world.send_n   (xsend.data(), count, partner_rank);
	world.receive_n(xrecv.data(), count);
	assert(xrecv[5] == 5);

	if(world.is_root()) {std::cerr<<"successfully completed"<<std::endl;}
	return 0;
}
```

This completes the replacement of the original MPI interface.
Further steps can be taken to exploit the safety provided by the library. 
For example, instead of using pointers from the dynamic arrays, we can use the iterators to describe the start of the sequences.

```cpp
...
	world.send_n   (xsend.begin(), xsend.size(), partner_rank);
	world.receive_n(xrecv.begin(), xrecv.size());
...
```
or use the range.

```cpp
...
	world.send   (xsend.begin(), xsend.end(), partner_rank);
	world.receive(xrecv.begin(), xrecv.end());
...
```

(Note that `_n` was dropped from the method name because we are using iterator ranges now.)

Finally, the end of the receiving sequence can be ommited in many cases since the information is contained in the message and the correctness can be ensured by the logic of the program.

```cpp
...
	world.send(xsend.begin(), xsend.end(), partner_rank);
	auto last = world.receive(xrecv.begin());  assert(last == xrecv.end()); 
...
```

After some rearrangement we obtain the final code, which is listed below.
We also replace separate calls by a single `send_receive` call which is optimized by the MPI system and more correct in this case, also we ensure "constness" of the sent values (`cbegin/cend`)).
There are no pointers being used in this final version.

```cpp
#include "../../mpi3/main.hpp"

#include<iostream>
#include<numeric>
#include<vector>

namespace bmpi3 = boost::mpi3;

int bmpi3::main(int /*argc*/, char ** /*argv*/, bmpi3::communicator world) {
	if(world.size()%2 == 1) {
	   if(world.is_root()) {std::cerr<<"Must be called with an even number of processes"<<std::endl;}
	   return 1;
	}

	std::vector<double> xsend(10); iota(begin(xsend), end(xsend), 0);
	std::vector<double> xrecv(xsend.size(), -1);

	world.send_receive(cbegin(xsend), cend(xsend), (world.rank()/2)*2 + (world.rank()+1)%2), begin(xrecv));

	assert(xrecv[5] == 5);
	if(world.is_root()) {std::cerr<<"successfully completed"<<std::endl;}
	return 0;
}
```
