#include "mxmultiply.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MASTER        0
#define LENGTH        1024
 

int main()
{ 
    int i, num_tasks, task_id, *scounts, *displs;
    size_t si, ei;
    double cpu_time;
    struct timespec start, end;
    float *a, *b, *c, *ba, *bb, *p;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    // start timer
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    scounts = calloc(num_tasks, sizeof(int));
    displs = calloc(num_tasks, sizeof(int));

    for(i = 0; i < num_tasks; i++)
    {
        si = gsif(LENGTH, i, num_tasks); 
        ei = geif(LENGTH, i, num_tasks);
        scounts[i] = LENGTH * (ei - si);
    }

    displs[1] = scounts[0];
    for(i = 2; i < num_tasks; i++)
        displs[i] = displs[i - 1] + scounts[i - 1];

    if(task_id == MASTER)
    {
        a = malloc(LENGTH * LENGTH * sizeof(float));
        b = malloc(LENGTH * LENGTH * sizeof(float));
        c = calloc(LENGTH * LENGTH, sizeof(float));
        rmxinitf(LENGTH, a, 8);
        rmxinitf(LENGTH, b, 5);
        mxtransposef(LENGTH, b);
    }

    // declare arrays for block matrices
    ba = malloc(scounts[task_id] * sizeof(float));
    bb = malloc(scounts[task_id] * sizeof(float));

    // scatter input matrices and compute product
    MPI_Scatterv(a, scounts, displs, MPI_FLOAT, ba, scounts[task_id], MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b, scounts, displs, MPI_FLOAT, bb, scounts[task_id], MPI_FLOAT, 0, MPI_COMM_WORLD);
    p = pmxmultiplyfs((scounts[task_id] / LENGTH), LENGTH, ba, LENGTH, (scounts[task_id] / LENGTH), bb);
    MPI_Reduce(p, c, (LENGTH * LENGTH), MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    // end timer
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    cpu_time = (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    cpu_time += (end.tv_sec - start.tv_sec);
    printf("execution time (task %d): %.5f s\n", task_id, cpu_time);

    MPI_Finalize(); 

    if(task_id == MASTER)
    {
        free(a);
        free(b);
        free(c); 
    }

    free(ba);
    free(bb);
    free(p);

    return 0;
}
