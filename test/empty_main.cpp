#if COMPILATION_INSTRUCTIONS
mpicxx $0 -o $0x&&mpirun -n 1 $0x $@&&rm $0x;exit
#endif
// © Alfredo A. Correa 2021

#include "../../mpi3/environment.hpp"

//namespace mpi3 = boost::mpi3;
//mpi3::environment env;

int main(){
}

