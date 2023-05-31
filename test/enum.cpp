// -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
// Copyright 2023 Alfredo A. Correa

#include <mpi3/communicator.hpp>
#include <mpi3/process.hpp>
#include <mpi3/main.hpp>

#include <boost/variant.hpp>

#include <complex>
#include <list>
#include <string>

namespace mpi3 = boost::mpi3;

struct WorkerStatus {
    int        createdEvents;
    int        skippedEvents;
    int        finishedEvents;
};

namespace Error {
enum Code { FAILURE = 0, SUCCESS = 1, RECOVERABLE = 2 };
}

namespace ErrorLong {
enum Code : unsigned long { FAILURE = 0, SUCCESS = 1, RECOVERABLE = 2 };
}

namespace ErrorClass {
enum class Code { FAILURE = 0, SUCCESS = 1, RECOVERABLE = 2 };
}

typedef unsigned long code_t;
enum ErrorCode : code_t { FAILURE = 0, SUCCESS = 1, RECOVERABLE = 2 };

auto mpi3::main(int /*argc*/, char** /*argv*/, mpi3::communicator world) -> int try {
	assert(world.size() > 1);

	switch(world.rank()) {
	break; case 0: {
		Error::Code const ec = Error::SUCCESS;
		world.send_n(&reinterpret_cast<int const&>(ec), 1, 1);
	};
	break; case 1: {
		Error::Code ec;
		world.receive_n(&reinterpret_cast<int&>(ec), 1, 0);
		assert(ec == Error::SUCCESS);
	};
	}

	switch(world.rank()) {
	break; case 0: {
		Error::Code const ec = Error::SUCCESS;
		world.send_n(&ec, 1, 1);
	};
	break; case 1: {
		Error::Code ec;
		world.receive_n(&ec, 1, 0);
		assert(ec == Error::SUCCESS);
	};
	}

	switch(world.rank()) {
	break; case 0: {
		Error::Code const ec = Error::SUCCESS;
		world.send_n(&ec, 1, 1);
	};
	break; case 1: {
		Error::Code ec;
		world.receive_n(&ec, 1);
		assert(ec == Error::SUCCESS);
	};
	}

	switch(world.rank()) {
	break; case 0: {
		Error::Code const ec = Error::SUCCESS;
		world[1] << ec;
	};
	break; case 1: {
		Error::Code ec;
		world[0] >> ec;
		assert(ec == Error::SUCCESS);
	};
	}

	switch(world.rank()) {
	break; case 0: {
		Error::Code const ec = Error::SUCCESS;
		world[1] << ec;
	};
	break; case 1: {
		Error::Code ec;
		world >> ec;
		assert(ec == Error::SUCCESS);
	};
	}

	switch(world.rank()) {
	break; case 0: {
		ErrorLong::Code const ec = ErrorLong::SUCCESS;
		world[1] << ec;
	};
	break; case 1: {
		ErrorLong::Code ec;
		world >> ec;
		assert(ec == ErrorLong::SUCCESS);
	};
	}

	switch(world.rank()) {
	break; case 0: {
		ErrorClass::Code const ec = ErrorClass::Code::SUCCESS;
		world[1] << ec;
	};
	break; case 1: {
		ErrorClass::Code ec;
		world >> ec;
		assert(ec == ErrorClass::Code::SUCCESS);
	};
	}

	return 0;
} catch(...) {
	return 1;
}
