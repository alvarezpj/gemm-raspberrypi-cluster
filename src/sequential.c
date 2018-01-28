#include "mxmultiply.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LENGTH        1024


int main()
{
    double cpu_time;
    struct timespec start, end;
    float *a = malloc(LENGTH * LENGTH * sizeof(float)); 
    float *b = malloc(LENGTH * LENGTH * sizeof(float));
    float *c = calloc(LENGTH * LENGTH, sizeof(float));

    rmxinitf(LENGTH, a, 8);
    rmxinitf(LENGTH, b, 5);

    // start timer
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);

    mxmultiplyf(LENGTH, a, b, c);

    // end timer
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

    cpu_time = (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    cpu_time += (end.tv_sec - start.tv_sec); 
    printf("execution time: %.5f s\n", cpu_time);

    free(a);
    free(b);
    free(c);

    return 0;
}