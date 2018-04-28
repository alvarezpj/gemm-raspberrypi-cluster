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



## Usage  

For compiling and deploying the code in this repository, you must run the usual GCC and MPI commands. To make life a lot easier, I have written multiple aliases in my [.bashrc](./conf/.bashrc) file. I also wrote a [makefile](./src/makefile). Before anything, however, you need to make sure that in your master node you have:

1. copied the *SETTINGS AND ALIAS DEFINITIONS FOR MASTER NODE* block from my [.bashrc](./conf/.bashrc) into yours.  
2. created a ```.mpiconfig``` folder in your home directory and added a [machine_file](./conf/machine_file) in it. A machine file is simply a plain text file containing a list of IP addresses corresponding to the IP addresses of all nodes in the cluster.   
3. added a [hosts_file](./conf/hosts_file) in ```~/.ssh```. If the directory does not exist, create it.  
4. set the ```GOMP_CPU_AFFINITY``` environment variable. This, however, should have been done after completing step 1.   

Now, for example, to compile and deploy [omp_mpi_neon.c](./src/omp_mpi_neon.c) you just run the following commands:  

```
# while in the gemm-raspberrypi-cluster directory
cd ./src
make omp_mpi_neon
# create a directory in a convenient location to keep all executables 
# this directory must be created in all nodes
mkdir ~/binaries
pssh mkdir /home/pi/binaries
# now copy the executable
cp ./omp_mpi_neon ~/binaries
cd ~/binaries
pscp ./omp_mpi_neon ~/home/pi/binaries
# run the executable
runmpi /home/pi/binaries/omp_mpi_neon 
```  

The script [collect_data.sh](./src/collect_data.sh) can be used to collect timing data by running an executable ten times for three different matrix sizes. By default, it tries matrix sizes 1024 x 1024, 2048 x 2048, and 4096 x 4096. For each matrix size, it compiles and runs the executable ten times. To use it, simply run ```./collect_data.sh <file name>```. For instance, ```./collect_data.sh omp_mpi_neon```.  



## Troubleshoot

Please look into the provided [.bashrc](./conf/.bashrc), [hosts_file](./conf/hosts_file), [machine_file](./conf/machine_file), [makefile](./src/makefile), and [collect_data.sh](./src/collect_data.sh) files if you are not sure about something. I hope the comments I wrote in each one are useful to you. If you encounter any bugs, please feel free to create an issue.  



## Notes  
  
* The programs in this repository were tested on a 12-node Raspberry Pi cluster running Raspbian Stretch with GCC 6.3 and MPICH 3.2.
* OpenMP requires the ```GOMP_CPU_AFFINITY``` environment variable to be set. See the included [.bashrc](./conf/.bashrc) file for more details on this.  
