#include "mxmultiply.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MASTER        0
#define LENGTH        1024
 

int main()
{ 
    int num_tasks, task_id;
    double cpu_time;
    struct timespec start, end;
    float *a = malloc(LENGTH * LENGTH * sizeof(float)); 
    float *b = malloc(LENGTH * LENGTH * sizeof(float));
    float *c = calloc(LENGTH * LENGTH, sizeof(float));
    float *p = calloc(LENGTH * LENGTH, sizeof(float));

    rmxinitf(LENGTH, a, 8);
    rmxinitf(LENGTH, b, 5);
 
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    // start timer
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    pmxmultiplyf(LENGTH, a, b, p, task_id, num_tasks);
    MPI_Reduce(p, c, (LENGTH * LENGTH), MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    // end timer
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    cpu_time = (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    cpu_time += (end.tv_sec - start.tv_sec);
    printf("execution time (task %d): %.5f s\n", task_id, cpu_time);

    MPI_Finalize();       

    free(a);
    free(b);
    free(c);
    free(p);

    return 0;
}