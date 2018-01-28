### GEMM optimized for Raspberry Pi clusters

assumptions: 
    * matrices are just (one-dimensional) arrays of floating point numbers
    * all matrices are square
    * the length of any matrix is a multiple of 4 
    * entries in a matrix are stored in column-major order

header **mxmultiply.h** in src/ provides four different levels of optimization for GEMM:
    1. multithreaded (OpenMP) 
    2. parallel (MPI) 
    3. vectorized (NEON)
    4. multithreaded, parallel and vectorized (OpenMP + MPI + NEON)

**note:** MPI calls are made inside a main() function

**note:** OpenMP requires the ```GOMP_CPU_AFFINITY``` environment variable to be set. Add the following line to your .bashrc or .zshrc file:

```bash
export GOMP_CPU_AFFINITY="0-3"
```

**note:** this code was tested on Raspbian Stretch with GCC 6.3 and MPICH 3.2

