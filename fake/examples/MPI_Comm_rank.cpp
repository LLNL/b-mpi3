#include "mpi.h"
#include <stdio.h>
int main(int argc, char *argv[])
{
    int rank, nprocs;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_NULL,&nprocs);
    MPI_Comm_rank(MPI_COMM_NULL,&rank);
    printf("Hello, world.  I am %d of %d\n", rank, nprocs);fflush(stdout);
    MPI_Finalize();
    return 0;
}
