/*  optimizing GEMM for a 12-node Raspberry Pi cluster

    assumptions: 
        * matrices are just (one-dimensional) arrays of floating point numbers
        * all matrices are square
        * the length of any matrix is a multiple of 4 
        * entries in a matrix are stored in column-major order
    
    this header provides four different levels of optimization for GEMM:
        1. multithreaded (OpenMP) 
        2. parallel (MPI) 
        3. vectorized (NEON)
        4. multithreaded, parallel and vectorized (OpenMP + MPI + NEON)

    note: MPI calls are made inside a main() function

    code tested on Raspbian Stretch with GCC 6.3 and MPICH 3.2 */

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <arm_neon.h>


// number of elements per vector register
#define VECREG_LEN                             4 
// vector load
#define VECLOD(ptr)                            vld1q_f32(ptr) 
// vector multiply by scalar
#define VMULSC(vecreg, scalar)                 vmulq_n_f32(vecreg, scalar)
// vector multiply accumulate
#define VMULAC(vecreg1, vecreg2, vecreg3)      vmlaq_f32(vecreg1, vecreg2, vecreg3)
// extract lanes from a vector
#define VEXTLN(vecreg, lane)                   vgetq_lane_f32(vecreg, lane)




/***********************************************************************************
*  Miscellaneous                                                                   *
***********************************************************************************/

/* initialize matrix */
void mxinitf(size_t len, float *mx, size_t mod)
{
    size_t i;

    for(i = 0; i < (len * len); i++)
        *(mx + i) = (float)((i * i) % mod);
}


/* initialize matrix with random floating point numbers in range [0.0, ubound] */
void rmxinitf(size_t len, float *mx, float ubound)
{
    size_t i;
    srand(time(0));

    for(i = 0; i < (len * len); i++)
        *(mx + i) = ((float)rand() / (float)RAND_MAX) * ubound;
}


/* print matrix to standard output */
void printmxf(size_t ncols, size_t nrows, float *mx, char omode) 
{
    size_t i, j;

    // print matrix in column-major order
    if(omode == 'c')
    {
        for(i = 0; i < ncols; i++)
        {    
            for(j = 0; j < nrows; j++) 
                printf("%-10.3f", *(mx + (nrows * i) + j));
       
            printf("\n");
        }
    }
    // print matrix in row-major order
    else if(omode == 'r')
    {
        for(i = 0; i < nrows; i++)
        {    
            for(j = 0; j < ncols; j++) 
                printf("%-10.3f", *(mx + (nrows * j) + i));
       
            printf("\n");
        }
    }
}


/* print matrix to file */
void fprintmxf(size_t ncols, size_t nrows, float *mx, char omode, char *str) 
{
    size_t i, j;
    FILE *file = fopen(str, "w");

    // print matrix in column-major order
    if(omode == 'c')
    {
        for(i = 0; i < ncols; i++)
        {    
            for(j = 0; j < nrows; j++) 
                fprintf(file, "%-10.3f", *(mx + (nrows * i) + j));
       
            fprintf(file, "\n");
        }
    }
    // print matrix in row-major order
    else if(omode == 'r')
    {
        for(i = 0; i < nrows; i++)
        {    
            for(j = 0; j < ncols; j++) 
                fprintf(file, "%-10.3f", *(mx + (nrows * j) + i));
       
            fprintf(file, "\n");
        }
    }

    fclose(file);
}


/* matrix transpose */
void mxtransposef(size_t len, float *mx)
{
    size_t i, j;
    float tmp;

    for(i = 0; i < len; i++)
    {
        for(j = (i + 1); j < len; j++)
        {
            tmp = *(mx + (len * j) + i);
            *(mx + (len * j) + i) = *(mx + (len * i) + j);
            *(mx + (len * i) + j) = tmp;
        }
    }
}


/** NOTE: the following two functions are used to partition matrices **/

/* get start index */
size_t gsif(size_t len, size_t task_id, size_t num_tasks)
{
    return ((task_id * len) / num_tasks);
}


/* get end index */
size_t geif(size_t len, size_t task_id, size_t num_tasks)
{
    return (((task_id + 1) * len) / num_tasks);
}




/***********************************************************************************
*  Sequential                                                                      *
***********************************************************************************/

/* general matrix multiply */
void mxmultiplyf(size_t len, float *mxa, float *mxb, float *mxc) 
{
    size_t i, j, k;

    for(i = 0; i < len; i++)
    {
        for(j = 0; j < len; j++)
        {
            for(k = 0; k < len; k++)
                //(*(mxc + (len * j) + i)) += (*(mxa + (len * k) + i)) * (*(mxb + (len * j) + k));
                (*(mxc + (len * i) + k)) += (*(mxa + (len * j) + k)) * (*(mxb + (len * i) + j));
        }  
    }
}




/***********************************************************************************
*  OpenMP                                                                          *
***********************************************************************************/

/* multithreaded general matrix multiply */ 
void mmxmultiplyf(size_t len, float *mxa, float *mxb, float *mxc) 
{
    size_t i, j, k;

    #pragma omp parallel num_threads(4)
    {
        #pragma omp for private(i, j, k) collapse(3)
        for(i = 0; i < len; i++)
        {
            for(j = 0; j < len; j++)
            {
                for(k = 0; k < len; k++)
                    (*(mxc + (len * i) + k)) += (*(mxa + (len * j) + k)) * (*(mxb + (len * i) + j));
            }  
        }
    }
}




/***********************************************************************************
*  MPI                                                                             *
***********************************************************************************/

/* parallel general matrix multiply */ 
void pmxmultiplyf(size_t len, float *mxa, float *mxb, float *mxc, size_t task_id, size_t num_tasks)
{
    size_t i, j, k, si, ei;

    si = (task_id * len) / num_tasks; 
    ei = ((task_id + 1) * len) / num_tasks;

    for(i = 0; i < len; i++)
    {
        for(j = si; j < ei; j++)
        {
            for(k = 0; k < len; k++)
                (*(mxc + (len * i) + k)) += (*(mxa + (len * j) + k)) * (*(mxb + (len * i) + j));
        }  
    }
}




/***********************************************************************************
*  NEON (SIMD)                                                                     *
***********************************************************************************/

/* vectorized general matrix multiply */
void vmxmultiplyf(size_t len, float *mxa, float *mxb, float *mxc)
{
    float32x4_t a, b, c;
    size_t i, j, k, reps = len / VECREG_LEN;
    float *mxap, *mxbp, *mxcp, tmp;

    // transpose matrix mxa
    for(i = 0; i < len; i++)
    {
        for(j = (i + 1); j < len; j++)
        {
            tmp = *(mxa + (len * j) + i);
            *(mxa + (len * j) + i) = *(mxa + (len * i) + j);
            *(mxa + (len * i) + j) = tmp;
        }
    }

    // multiply
    for(i = 0; i < len; i++)
    {
        mxap = mxa;

        for(j = 0; j < len; j++)
        {
            c = VMULSC(c, 0.0); 
            mxbp = mxb + (len * i);

            for(k = 0; k < reps; k++)
            {
                a = VECLOD(mxap); 
                b = VECLOD(mxbp);
                c = VMULAC(c, a, b); 
                mxap += VECREG_LEN;
                mxbp += VECREG_LEN;
            }

            mxcp = mxc + (len * i) + j;
            // compute entry
            tmp = VEXTLN(c, 0) + VEXTLN(c, 1) + VEXTLN(c, 2) + VEXTLN(c, 3);
            // store entry in result matrix
            *(mxcp) = tmp; 
        } 
    }
}




/***********************************************************************************
*  OpenMP + MPI + NEON                                                             *
***********************************************************************************/

/* multithreaded, parallel, and vectorized general matrix multiply */
float *mpvmxmultiplyf(size_t len, float *mxa, float *mxb, int task_id, int num_tasks, size_t *ncols)
{
    size_t si, ei;
    float *buf;

    // get block from mxb (i.e. get start and end columns)
    si = (task_id * len) / num_tasks;
    ei = ((task_id + 1) * len) / num_tasks;
    // compute number of columns in partial result (block)
    *ncols = ei - si;
    buf = calloc(len * (*ncols), sizeof(float));

    #pragma omp parallel firstprivate(len, mxa, mxb, buf, ei, si) num_threads(4)
    { 
        float32x4_t a, b, c;
        size_t i, j, k, reps = len / VECREG_LEN;
        float *mxap, *mxbp, *bufp, tmp;

        // transpose matrix mxa
        #pragma omp for
        for(i = 0; i < len; i++)
        {
            for(j = (i + 1); j < len; j++)
            {
                tmp = *(mxa + (len * j) + i);
                *(mxa + (len * j) + i) = *(mxa + (len * i) + j);
                *(mxa + (len * i) + j) = tmp;
            }
        }

        // multiply
        #pragma omp for 
        for(i = si; i < ei; i++)
        {
            mxap = mxa;

            for(j = 0; j < len; j++)
            {
                c = VMULSC(c, 0.0); 
                mxbp = mxb + (len * i);

                for(k = 0; k < reps; k++)
                {
                    a = VECLOD(mxap); 
                    b = VECLOD(mxbp);
                    c = VMULAC(c, a, b); 
                    mxap += VECREG_LEN;
                    mxbp += VECREG_LEN;
                }

                bufp = buf + (len * (i - si)) + j;
                // compute entry
                tmp = VEXTLN(c, 0) + VEXTLN(c, 1) + VEXTLN(c, 2) + VEXTLN(c, 3);
                // store entry in result matrix
                *(bufp) = tmp; 
            } 
        }
    }

    return buf;
}
