#ifdef COMPILATION// -*-indent-tabs-mode:t;c-basic-offset:4;tab-width:4-*-
$CXX $0 -o $0x &&$0x&&rm $0x;exit
#endif
// © Alfredo A. Correa 2019-2020

#ifndef MPI3_CONFIG_NODISCARD_HPP
#define MPI3_CONFIG_NODISCARD_HPP

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(name) 0
#endif

#if (__has_cpp_attribute(nodiscard)) && (__cplusplus>=201703L)
	#define NODISCARD(MsG) [[nodiscard]]
	#if (__has_cpp_attribute(nodiscard)>=201907) && (__cplusplus>=201703L)
		#define NODISCARD(MsG) [[nodiscard(MsG)]]
	#endif
#elif __has_cpp_attribute(gnu::warn_unused_result)
	#define NODISCARD(MsG) [[gnu::warn_unused_result]]
#else
	#define NODISCARD(MsG)
#endif

#if not __INCLUDE_LEVEL__ // _TEST_MPI3_CONFIG_NODISCARD

NODISCARD("because...") int f(){return 5;}
//[[nodiscard]] int g(){return 5;} // ok in g++ -std=c++14

int main(){
	int i; 
	i = f(); // ok
//	f();  // warning
	i += 1;
}
#endif
#endif


