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
    float *a, *b;
    float *c = calloc(LENGTH * LENGTH, sizeof(float));
    float *p = calloc(LENGTH * LENGTH, sizeof(float));

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);

    if(task_id == MASTER)
    {
        a = malloc(LENGTH * LENGTH * sizeof(int));
        b = malloc(LENGTH * LENGTH * sizeof(int));
        mxinitf(LENGTH, a, 8);
        mxinitf(LENGTH, b, 5);
    }
    // start timer
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    mxtransposef(LENGTH, b);

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

    if(task_id != MASTER)
    {
        a = malloc(LENGTH * scounts[task_id] * sizeof(float));
        b = malloc(LENGTH * scounts[task_id] * sizeof(float));
    }

    MPI_Scatterv(a, scounts, displs, MPI_FLOAT, a, scounts[task_id], MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Scatterv(b, scounts, displs, MPI_FLOAT, b, scounts[task_id], MPI_FLOAT, 0, MPI_COMM_WORLD);
    //MPI_Bcast(a, (LENGTH * LENGTH), MPI_FLOAT, 0, MPI_COMM_WORLD);   
    //MPI_Bcast(b, (LENGTH * LENGTH), MPI_FLOAT, 0, MPI_COMM_WORLD);
    pmxmultiplyfs();
    MPI_Reduce(p, c, (LENGTH * LENGTH), MPI_FLOAT, MPI_SUM, 0, MPI_COMM_WORLD);

    // end timer
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    cpu_time = (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    cpu_time += (end.tv_sec - start.tv_sec);
    printf("execution time (task %d): %.5f s\n", task_id, cpu_time);
    if(task_id == MASTER)
        fprintmxf(LENGTH, LENGTH, c, 'r', "mpi_b.mx");

    MPI_Finalize();       

    free(a);
    free(b);
    free(c);
    free(p);

    return 0;
}
