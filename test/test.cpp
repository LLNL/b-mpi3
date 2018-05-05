#if COMPILATION_INSTRUCTIONS
c++ -std=c++11 -O3 -Wall -Wextra -Wfatal-errors $0 -o $0x.x -lstdc++fs && $0x.x $@  ; rm -f $0x.x; exit
#endif


#include<iostream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

int main(){
	for(fs::path const& p: fs::directory_iterator("./")){
		if(p == "./test.cpp") continue;
		std::cout << p << '\n';
		std::system(("sh " + p.string()).c_str());
	}
}
