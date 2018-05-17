#!/bin/bash

######################################################################
#                                                                    #
# Bash script to collect sample data from (Raspberry Pi) cluster. It #
# works by compiling and running the matrix multiplication programs  #
# "sequential.c", "omp.c", and "omp_mpi_neon.c" and changing the     #
# following three parameters:                                        #
#                                                                    #
#     1. number of threads: 1, 2, 3, and 4                           #
#     2. number of nodes: 1, 3, 6, 9, and 12                         #
#     3. matrix size: 1024 x 1024, 2048 x 2048, and 4096 x 4096      #
#                                                                    #
# for each permutation of parameters, the program is run ten times.  #
#                                                                    #
######################################################################


# set up variables for "sequential.c"
file="sequential.c"
executable="sequential"
msizes=(1024 2048 4096)

# compile and execute program
# loop structure: matrix sizes --> number of repetitions 
for i in ${msizes[@]}
do
    sed -i "s/#define LENGTH.*/#define LENGTH        $i/" ./$file
    make -s $executable
    echo "Results corresponding to [nnodes=1, nthreads=1, msize=$i]"

    for j in {1..10}
    do
        echo "trial $j"
        ./sequential
        sleep 120
    done
done

# set up variables for "omp.c"
header="mxmultiply.h"
file="omp.c"
executable="omp"

# compile and execute program
# loop structure: number of threads --> matrix sizes --> number of repetitions 
for i in {2..4}
do
    sed -i "s/#define NUMTHREADS.*/#define NUMTHREADS                             $i/" ./$header

    for j in ${msizes[@]}
    do
        sed -i "s/#define LENGTH.*/#define LENGTH        $j/" ./$file
        make -s $executable
        echo -e "\nResults corresponding to [nnodes=1, nthreads=$i, msize=$j]"

        for k in {1..10}
        do
            echo "trial $k"
            ./$executable
            sleep 120
        done 
    done
done 

# set up variables for "omp_mpi_neon.c"
directory=`pwd`
file="omp_mpi_neon.c"
executable="omp_mpi_neon"
nnodes=(3 6 9 12)

# create directories in slave nodes
echo
parallel-ssh -i -h ~/.ssh/hosts_file mkdir -p $directory

# compile and execute program
# loop structure: number of nodes --> number of threads --> matrix sizes --> number of repetitions 
for i in ${nnodes[@]}
do
    for j in {1..4}
    do
        sed -i "s/#define NUMTHREADS.*/#define NUMTHREADS                             $j/" ./$header

        for k in ${msizes[@]}
        do
            sed -i "s/#define LENGTH.*/#define LENGTH        $k/" ./$file
            make -s $executable
            echo
            parallel-scp -v -h ~/.ssh/hosts_file ./$executable $directory
            echo -e "\nResults corresponding to [nnodes=$i, nthreads=$j, msize=$k]"

            for l in {1..10}
            do
                echo "trial $l"
                mpirun -np $i -machinefile ~/.mpiconfig/nodes $directory/$executable 
                sleep 120
            done 
        done
    done 
done

# clean up
make -s clean

