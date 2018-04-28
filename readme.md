# General Matrix Multiplication Optimized For Raspberry Pi Clusters  



![A 12-node Raspberry Pi cluster](./assets/cluster.jpg)
*A 12-node Raspberry Pi cluster*



## Introduction

This repository provides a set of functions for computing the product of two matrices. These functions are intended to be run on a cluster built with Raspberry Pi computers. Header file [mxmultiply.h](./src/mxmultiply.h) supplies four different levels of optimization, each implemented as a separate function, on the general matrix multiply algorithm:  

1. multithreaded (OpenMP)  
2. parallel (MPI)  
3. vectorized (NEON)  
4. multithreaded, parallel, and vectorized (OpenMP + MPI + NEON)  

To facilitate development, all functions were written with the following assumptions in mind:   
* matrices are just (one-dimensional) arrays of floating point numbers.  
* all matrices are square. 
* the length of any matrix is a multiple of 4.   
* entries in a matrix are stored in column-major order.  

There are also sample programs which demonstrate each of the aforementioned optimizations. These are found in the ```scr``` directory. After execution, all programs print out their running time in seconds.  



## Cluster Setup

A cluster should have a *master* node. This node is used as a hub; all development is done here: we write, compile, and deploy binaries from this node. All other nodes in the cluster are referred to as *slave* nodes. They are mainly used for computational purposes. To maximize cluster usage, however, the code here also utilizes the master node as a computational unit. Details on how to set up your own cluster can be found in this [excelent guide](./howto/How_to_Make_a_Raspberry_Pi_Cluster-Mike_Garcia.pdf) written by Mike Garcia.   

 

## Dependencies

To run the sample programs in this repository, you will need to install both GCC and MPICH on all nodes in the cluster. On the latest Raspbian (Stretch), GCC comes installed by default. So, you will only need to install MPICH via ```sudo apt-get install gcc mpich```. See this [guide](How_to_Make_a_Raspberry_Pi_Cluster-Mike_Garcia.pdf) for more details.  

To easily deploy binaries and issue commands to all slave nodes in the cluster, you should install the parallel SSH toolset. You can do so by running ```sudo apt-get install pssh```. If you have a big cluster with many nodes, you may want to write a hosts file to use with parallel-ssh and parallel-scp. I provide a sample file for a 12-node cluster [here](./conf/hosts_file). Note that this file can be easily adapted for clusters of any size.  



## Usage (to complete)

For compiling and deploying the code in this repository, you must run the usual GCC and MPI commands. To make life a lot easier, I have written multiple aliases in my [.bashrc](./conf/.bashrc) file. I also wrote a [makefile](./src/makefile). So, for compiling and deploying [omp_mpi_neon.c](./src/omp_mpi_neon.c) you just run the following commands in order:  

```
# while in the gemm-raspberrypi-cluster directory
cd ./src
make omp_mpi_neon
runmpi ./omp_mpi_neon
```  

The script [collect_data.sh](./src/collect_data.sh) can be used to collect data



## Notes
  
* The programs in this repository were tested on a 12-node Raspberry Pi cluster running Raspbian Stretch with GCC 6.3 and MPICH 3.2.
* OpenMP requires the ```GOMP_CPU_AFFINITY``` environment variable to be set. See the included [.bashrc](./conf/.bashrc) file for more details on this.  
