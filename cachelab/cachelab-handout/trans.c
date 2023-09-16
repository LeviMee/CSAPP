/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (M == 32 && N == 32)
    {
        for (int i = 0; i < 32; i += 8)
        {
            for (int j = 0; j < 32; j += 8)
            {
                for (int ii = 0; ii < 8; ++ii)
                {
                    int r0 = A[i + ii][j];
                    int r1 = A[i + ii][j + 1];
                    int r2 = A[i + ii][j + 2];
                    int r3 = A[i + ii][j + 3];
                    int r4 = A[i + ii][j + 4];
                    int r5 = A[i + ii][j + 5];
                    int r6 = A[i + ii][j + 6];
                    int r7 = A[i + ii][j + 7];

                    B[j][i + ii] = r0;
                    B[j + 1][i + ii] = r1;
                    B[j + 2][i + ii] = r2;
                    B[j + 3][i + ii] = r3;
                    B[j + 4][i + ii] = r4;
                    B[j + 5][i + ii] = r5;
                    B[j + 6][i + ii] = r6;
                    B[j + 7][i + ii] = r7;
                }
            }
        }
    }

    if (M == 64 && N == 64)
    {

    }

    if (M == 61 && N == 67)
    {
        int i, j;
        for (i = 0; i < 64; i += 8)
        {
            for (j = 0; j < 56; j += 8)
            {
                for (int ii = 0; ii < 8; ++ii)
                {
                    int r0 = A[i + ii][j];
                    int r1 = A[i + ii][j + 1];
                    int r2 = A[i + ii][j + 2];
                    int r3 = A[i + ii][j + 3];
                    int r4 = A[i + ii][j + 4];
                    int r5 = A[i + ii][j + 5];
                    int r6 = A[i + ii][j + 6];
                    int r7 = A[i + ii][j + 7];

                    B[j][i + ii] = r0;
                    B[j + 1][i + ii] = r1;
                    B[j + 2][i + ii] = r2;
                    B[j + 3][i + ii] = r3;
                    B[j + 4][i + ii] = r4;
                    B[j + 5][i + ii] = r5;
                    B[j + 6][i + ii] = r6;
                    B[j + 7][i + ii] = r7;
                }
            }
        }

        for (i = 64; i < N; ++i)
        {
            for (j = 0; j < M; ++j)
            {
                int tmp = A[i][j];
                B[j][i] = tmp;
            }
        }
        for (i = 0; i < N; ++i)
        {
            for (j = 56; j < M; ++j)
            {
                int tmp = A[i][j];
                B[j][i] = tmp;
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 
char trans_test_[] = "test";
void transpose_test_(int M, int N, int A[N][M], int B[M][N])
{


}


/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) 
    {
        for (j = 0; j < M; j++) 
        {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

