#!/bin/bash

######################################################################
#                                                                    #
# Bash script to collect sample data from (Raspberry Pi) cluster. It #
# works by compiling and running the matrix multiplication program   #
# "omp_mpi_neon.c" and changing the following three parameters:      #
#                                                                    #
#     1. number of threads: 1, 2, 3, and 4                           #
#     2. number of nodes: 1, 3, 6, 9, and 12                         #
#     3. matrix size: 1024 x 1024, 2048 x 2048, and 4096 x 4096      #
#                                                                    #
# for each permutation of parameters, the program is run ten times.  #
#                                                                    #
######################################################################


# set up variables
directory=`pwd`
header="mxmultiply.h"
file="omp_mpi_neon.c"
executable="omp_mpi_neon"
nnodes=(3 6 9 12)
msizes=(1024 2048 4096)

# create directories in slave nodes
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
                echo -n "$l. "
                mpirun -np $i -machinefile ~/.mpiconfig/nodes $directory/$executable 
                sleep 120
            done 
        done
    done 
done

# clean up
make -s clean

