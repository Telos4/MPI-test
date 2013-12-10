all:
	mpic++ -Wall -g -c main.cpp  -o main.o
	mpic++ -o mpi_test main.o

clean:
	rm *.o mpi_test
