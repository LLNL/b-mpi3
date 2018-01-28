all : abort.x MPI_Send.x MPI_Recv.x
	../mpirun -np 1 ./abort.x
	../mpirun -np 1 ./MPI_Send.x
	../mpirun -np 1 ./MPI_Recv.x
	rm -f $^

abort.x : abort.cpp
	../mpic++ -I../ abort.cpp -o abort.x

MPI_Send.x : MPI_Send.cpp
	../mpic++ -I../ MPI_Send.cpp -o MPI_Send.x

MPI_Recv.x : MPI_Recv.cpp
	../mpic++ -I../ MPI_Recv.cpp -o MPI_Recv.x

