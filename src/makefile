.POSIX:
.SUFFIXES:


CC      = gcc
CFLAGS  = -W -mcpu=cortex-a53 -mfpu=neon-fp-armv8


all: sequential omp mpi neon omp_mpi_neon


sequential: sequential.o
	$(CC) -o sequential sequential.o
sequential.o: sequential.c mxmultiply.h
	$(CC) -c $(CFLAGS) sequential.c


omp: omp.o
	$(CC) -fopenmp -o omp omp.o
omp.o: omp.c mxmultiply.h
	$(CC) -c -fopenmp $(CFLAGS) omp.c 


mpi: mpi.o
	mpicc -o mpi mpi.o
mpi.o: mpi.c mxmultiply.h
	mpicc -c $(CFLAGS) mpi.c


neon: neon.o
	$(CC) -o neon neon.o
neon.o: neon.c mxmultiply.h
	$(CC) -c $(CFLAGS) neon.c


omp_mpi_neon: omp_mpi_neon.o
	mpicc -fopenmp -o omp_mpi_neon omp_mpi_neon.o
omp_mpi_neon.o: omp_mpi_neon.c mxmultiply.h
	mpicc -c -fopenmp $(CFLAGS) omp_mpi_neon.c


clean:
	rm -f sequential sequential.o omp omp.o mpi mpi.o neon neon.o omp_mpi_neon omp_mpi_neon.o
