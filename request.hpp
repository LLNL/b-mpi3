#if COMPILATION_INSTRUCTIONS // -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4;autowrap:nil;-*-
(echo "#include\""$0"\"" > $0x.cpp) && mpic++ -O3 -std=c++14 -Wall `#-Wfatal-errors` -D_TEST_BOOST_MPI3_REQUEST $0x.cpp -o $0x.x && time mpirun -np 4 $0x.x $@ && rm -f $0x.x $0x.cpp; exit
#endif
#ifndef BOOST_MPI3_REQUEST_HPP
#define BOOST_MPI3_REQUEST_HPP

#include "../mpi3/detail/call.hpp"
#include "../mpi3/detail/iterator.hpp" // detail::data

#include "../mpi3/status.hpp"

// #define OMPI_SKIP_MPICXX 1  // https://github.com/open-mpi/ompi/issues/5157
#include<mpi.h>

#include<stdexcept>
#include<vector>

namespace boost {
namespace mpi3 {

struct request {
	MPI_Request impl_ = MPI_REQUEST_NULL;  // NOLINT(misc-non-private-member-variables-in-classes) TODO(correaa)
	request() = default;
	request(request const& other) = delete;

 public:
	request(request&& other) noexcept : impl_{std::exchange(other.impl_, MPI_REQUEST_NULL)}{}

	request& operator=(request const&) = delete;
	request& operator=(request&& other) noexcept {
		request(std::move(other)).swap(*this);
		return *this;
	}
	bool completed() const{
		int ret = -1;
		MPI_Request_get_status(impl_, &ret, MPI_STATUS_IGNORE);
		return ret != 0;
	}
	status get_status() const{
		status ret;  // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init) delayed initialization
		int ignore = -1;
		MPI_Request_get_status(impl_, &ignore, &ret.impl_);
		return ret;
	}
	void swap(request& other){std::swap(impl_, other.impl_);}
	void cancel(){MPI_Cancel(&impl_);}
	bool valid() const{return impl_ != MPI_REQUEST_NULL;}
	~request() noexcept {  // TODO(correaa) check it can be no noexcept and cancellable
		try {
			wait();
			if(impl_ != MPI_REQUEST_NULL) {MPI_Request_free(&impl_);}
		} catch(...) {
			std::terminate();
		}
	}
	void wait() {
		if(impl_ != MPI_REQUEST_NULL) {
			MPI_(Wait)(&impl_, MPI_STATUS_IGNORE);
		}
	}
	status get() {
		status ret;  // NOLINT(cppcoreguidelines-pro-type-member-init,hicpp-member-init) delayed initialization
		int s = MPI_Wait(&impl_, &ret.impl_);
		if(s != MPI_SUCCESS) {throw std::runtime_error("cannot wait on request");}
		return ret;
	}
	void start(){MPI_(Start)(&impl_);}
	status test() const{return get_status();}
};

inline std::vector<status> test_some(std::vector<request> const& requests) {
	int outcount = -1;
	std::vector<int> ignore(requests.size());
	std::vector<status> ret(requests.size()); 
	int s = MPI_Testsome(requests.size(), const_cast<MPI_Request*>(&(requests.data()->impl_)), &outcount, ignore.data(), &(ret.data()->impl_));
	if(s != MPI_SUCCESS) {throw std::runtime_error{"cannot test some"};}
	return ret;
}

inline std::vector<int> completed_some(std::vector<request> const& requests) {
	int outcount = -1;
	std::vector<int> ret(requests.size());
	int s = MPI_Testsome(requests.size(), const_cast<MPI_Request*>(&(requests.data()->impl_)), &outcount, ret.data(), MPI_STATUSES_IGNORE);
	if(s != MPI_SUCCESS) {throw std::runtime_error("cannot completed some");}
	ret.resize(outcount);
	return ret;
}

template<class ContRequestIterator, class Size>
void wait_all_n(ContRequestIterator it, Size n){
	MPI_Waitall(n, &detail::data(it)->impl_, MPI_STATUSES_IGNORE);
}

template<class ContRequestIterator>
void wait_all(ContRequestIterator it1, ContRequestIterator it2){
	wait_all_n(it1, std::distance(it1, it2));
}

template<class... Args>
void wait(Args&&... args){
	auto move_impl = [](request&& r)->MPI_Request{	MPI_Request ret = r.impl_;
		r.impl_ = MPI_REQUEST_NULL;
		return ret;
	};
	std::vector<MPI_Request> v{move_impl(std::move(args))...};
	MPI_Waitall(v.size(), v.data(), MPI_STATUSES_IGNORE);
}

template<class ContiguousIterator, class Size>
ContiguousIterator wait_any_n(ContiguousIterator it, Size n){
	int index = -1;
	int s = MPI_Waitany(n, &detail::data(it)->impl_, &index, MPI_STATUS_IGNORE);
	if(s != MPI_SUCCESS) throw std::runtime_error("cannot wait any");
	return it + index;
}

template<class ContiguousIterator>
ContiguousIterator wait_any(ContiguousIterator first, ContiguousIterator last){
	return wait_any_n(first, std::distance(first, last));
}

template<class ContiguousIterator, class Size>
std::vector<int> wait_some_n(ContiguousIterator it, Size n){
	int outcount = -1;
	std::vector<int> indices(n);
	int s = MPI_Waitsome(n, &detail::data(it)->impl_, &outcount, indices.data(), MPI_STATUSES_IGNORE);
	if(s != MPI_SUCCESS) throw std::runtime_error("cannot wait some");
	indices.resize(outcount);
	return indices;
}

template<class ContiguousIterator>
std::vector<int> wait_some(ContiguousIterator first, ContiguousIterator last){
	return wait_some_n(first, std::distance(first, last));
}

}  // end namespace mpi3
}  // end namespace boost

#ifdef _TEST_BOOST_MPI3_REQUEST

#include "../mpi3/environment.hpp"
#include<iostream>

using std::cout;
namespace mpi3 = boost::mpi3;

int main(int argc, char** argv) {
	mpi3::environment env(argc, argv);
	std::vector<int> buf(10);

#if 0
//	mpi3::send_
	mpi3::request r = env.world().send_init_n(buf.begin(), buf.size(), 0);

	std::vector<int> rbuf(10);
	if(env.world().rank() == 0){
		std::vector<mpi3::request> rr;//(env.world().size());
		for(int i = 0; i != env.world().size(); ++i)
			rr.emplace_back(env.world().ireceive(rbuf.begin(), rbuf.end(), i));
		r.start();
		r.wait();
		wait_all(rr.begin(), rr.end());
	}else{
		r.start();
		r.wait();
	}
#endif

#if 0
	if(env.world().rank() == 0){
	//	mpi3::receive_
		mpi3::request r = env.world().receive_init(rbuf.begin(), rbuf.end());
		mpi3::request sr = env.world().isend(buf.begin(), buf.end(), 0);
		for(int i = 0; i != env.world().size(); ++i){
			r.start();
			r.wait();
		}
		sr.wait();
	}else{
		env.world().send(buf.begin(), buf.end(), 0);
	}
#endif

	return 0;
}
#endif
#endif
