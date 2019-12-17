# General Matrix Multiplication Optimized for Raspberry Pi Clusters  



![A 12-node Raspberry Pi cluster](./assets/cluster.jpg)
*A 12-node Raspberry Pi cluster*



## Introduction  

This repository provides a set of functions for computing the product of two matrices. These functions are intended to be run on a cluster built with Raspberry Pi computers. Header file [mxmultiply.h](./src/mxmultiply.h) supplies four different levels of optimization, each implemented as a separate function, on the general matrix multiply algorithm:  

1. multithreaded (OpenMP)  
2. parallel (MPI)  
3. vectorized (NEON)  
4. multithreaded, parallel, and vectorized (OpenMP + MPI + NEON)  

To facilitate development, all functions were written with the following assumptions in mind:   
* matrices are just (one-dimensional) arrays of floating point numbers (float type).  
* all matrices are squared.  
* the length of any matrix is a multiple of 4.  
* entries in a matrix are stored in column-major order.  

There are also sample programs which demonstrate each of the aforementioned levels of optimization in the ```scr``` directory. Namely, these are [omp.c](./src/omp.c), [mpi.c](./src/mpi.c), [neon.c](./src/neon.c), and [omp_mpi_neon.c](./src/omp_mpi_neon.c). After execution, each of these programs prints out their running time in seconds. Additionally, there is one last [program](./src/sequential.c) which computes matrix multiplication in a sequential manner.  



## Cluster Setup  

A cluster should have a *master* node. This node is used as a hub; all development is done here: we write, compile, and deploy binaries from this node. All other nodes in the cluster are referred to as *slave* nodes. They are mainly used for computational purposes. To maximize cluster usage, however, the code in this repository also utilizes the master node as a computational unit. Details on how to set up your own cluster can be found in this [excelent guide](./howto/How_to_Make_a_Raspberry_Pi_Cluster-Mike_Garcia.pdf) written by Mike Garcia.  



## Cluster Performance

To assess the overall speed of the cluster, I ran the [HPL benchmark](http://www.netlib.org/benchmark/hpl/) (version hp1-2.2). It was configured to work with MPICH and BLAS (Basic Linear Algebra Subprograms). For a single problem size of 17,400 with just one process grid of 3 rows and 4 columns, the results indicated that this cluster was able to process 2.364 GFLOPS. To obtain this value, I ran the benchmark three times and took the average of the resuls.  



## Dependencies  

To run the sample programs in this repository, you will need to install both GCC and MPICH on all nodes in the cluster. On the latest Raspbian (Stretch, by the time this was written), GCC comes installed by default. So, you will only need to install MPICH via ```sudo apt-get install gcc mpich```. See this [guide](How_to_Make_a_Raspberry_Pi_Cluster-Mike_Garcia.pdf) for more details.  

To easily deploy binaries and issue commands to all slave nodes in the cluster, you should install the parallel SSH toolset. You can do so by running ```sudo apt-get install pssh```. If you have a big cluster with many nodes, you may want to write a hosts file to use with parallel-ssh and parallel-scp. I provided a sample file for a 12-node cluster [here](./conf/hosts_file). Note that this file can be easily adapted for clusters of any size.  



## Usage  

To compile and deploy the code in this repository, you must run the usual GCC and MPI commands. To make life a lot easier, I have written multiple aliases in my [.bashrc](./conf/.bashrc) configuration file. I also wrote a [makefile](./src/makefile). Before anything, however, you need to make sure that in your master node you have:

1. copied the *SETTINGS AND ALIAS DEFINITIONS FOR MASTER NODE* block from my [.bashrc](./conf/.bashrc) into yours. This block is at the bottom of the file.   
2. created a ```.mpiconfig``` folder in your home directory and added a [machine_file](./conf/machine_file) in it. A machine file is simply a plain text file containing a list of IP addresses corresponding to the IP addresses of all nodes in the cluster.   
3. added a [hosts_file](./conf/hosts_file) in ```~/.ssh```. If the directory does not exist, create it.  
4. set the ```GOMP_CPU_AFFINITY``` environment variable. Do not worry about this if you completed step 1. You can read more about why we set this variable [here](https://gcc.gnu.org/onlinedocs/libgomp/GOMP_005fCPU_005fAFFINITY.html).   

Now, for example, to compile and deploy [omp_mpi_neon.c](./src/omp_mpi_neon.c) run the following commands:  

```
# while in the gemm-raspberrypi-cluster directory
cd ./src
make omp_mpi_neon
# create a directory in a convenient location to keep all executables 
# this directory must be created in all nodes
mkdir ~/binaries
pssh mkdir /home/pi/binaries
# download the executable to all nodes in cluster 
cp ./omp_mpi_neon ~/binaries
cd ~/binaries
pscp ./omp_mpi_neon ~/home/pi/binaries
# run the executable
runmpi /home/pi/binaries/omp_mpi_neon 
```  

The steps for compiling and running [mpi.c](./src/mpi.c) are similar. The remaining programs, which do not use MPI, can be compiled and run on the master node. For [neon.c](./src/neon.c), simply:  

```
# while in the gemm-raspberrypi-cluster directory
make neon
./neon
```

The scripts [sample-collector-1.sh](./src/sample-collector-1.sh) and [sample-collector-2.sh](./src/sample-collector-2.sh) can be used to collect program data (execution times). They work by running a program ten times for three different matrix sizes. By default, both scripts try sizes 1024 x 1024, 2048 x 2048, and 4096 x 4096. To use any script, simply run ```./collect_data.sh <file name>```. For instance, 

```
./src/collect_data.sh omp_mpi_neon.c
```

Here, note that [sample-collector-1.sh](./src/sample-collector-1.sh) is intended for programs that make use of MPI. That is, only use this script for [mpi.c](./src/mpi.c) and [omp_mpi_neon.c](./src/omp_mpi_neon.c). For all other programs ([sequential.c](./src/sequential.c), [omp.c](./src/omp.c), and [neon.c](./src/neon.c)) use [sample-collector-2.sh](./src/sample-collector-2.sh).  



## Troubleshoot

Please look into the provided [.bashrc](./conf/.bashrc), [hosts_file](./conf/hosts_file), [machine_file](./conf/machine_file), [makefile](./src/makefile), [sample-collector-1.sh](./src/sample-collector-1.sh), and [sample-collector-2.sh](./src/sample-collector-2.sh) files if you are not sure about something. I hope the comments I wrote in each one are useful to you. If you encounter any bugs, please feel free to create an issue.  



## Notes  
  
* The programs in this repository were tested on a 12-node Raspberry Pi cluster running Raspbian Stretch with GCC 6.3 and MPICH 3.2.
* OpenMP requires the ```GOMP_CPU_AFFINITY``` environment variable to be set. See the included [.bashrc](./conf/.bashrc) file for more details on this.  
