#include "mxmultiply.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define MASTER        0
#define LENGTH        1024


int main()
{ 
    int i, num_tasks, task_id, *rcounts, *displs; 
    size_t si, ei;
    double cpu_time;
    struct timespec start, end;
    float *buffer, *b, *c, *tmx;
    float *a = malloc(LENGTH * LENGTH * sizeof(float));

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    if(task_id == MASTER)
    {
        b = malloc(LENGTH * LENGTH * sizeof(float)); 
        c = calloc(LENGTH * LENGTH, sizeof(float));
        rmxinitf(LENGTH, a, 8);
        rmxinitf(LENGTH, b, 5);
    }

    // start timer
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);

    rcounts = calloc(num_tasks, sizeof(int));
    displs = calloc(num_tasks, sizeof(int));

    for(i = 0; i < num_tasks; i++)
    {
        si = gsif(LENGTH, i, num_tasks);
        ei = geif(LENGTH, i, num_tasks);
        rcounts[i] = LENGTH * (ei - si); 
    }

    displs[1] = rcounts[0];
    for(i = 2; i < num_tasks; i++) 
        displs[i] = displs[i - 1] + rcounts[i - 1]; 

    tmx = malloc(rcounts[task_id] * sizeof(float));

    MPI_Bcast(a, (LENGTH * LENGTH), MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b, rcounts, displs, MPI_FLOAT, tmx, rcounts[task_id], MPI_FLOAT, 0, MPI_COMM_WORLD);
    // multiply matrices
    buffer = mpvmxmultiplyfs(LENGTH, a, (rcounts[task_id] / LENGTH), tmx);
    MPI_Gatherv(buffer, rcounts[task_id], MPI_FLOAT, c, rcounts, displs, MPI_FLOAT, 0, MPI_COMM_WORLD);

    // end timer
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &end);

    cpu_time = (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    cpu_time += (end.tv_sec - start.tv_sec);
    printf("execution time (task %d): %.5f s\n", task_id, cpu_time);

    MPI_Finalize();       

    if(task_id == MASTER)
    {
        free(b);
        free(c);
    }

    free(a);
    free(buffer);
    free(tmx);

    return 0;
}
