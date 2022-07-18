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
void transpose_submit(int M, int N, int A[N][M], int B[M][N]) {
    int block_len = (1 << 8) / M;
    int n_block = N / block_len;
    if(N % block_len > 0)   n_block++;
    int m_block = M / block_len;
    if(M % block_len > 0)   m_block++;
    for(int ib = 0; ib < n_block; ib++) {
        for(int jb = 0; jb < m_block; jb++) {
            if(ib == jb) {  // blocks on the diagonal
                int tmp[block_len];
                for(int i=ib*block_len; i<(ib+1)*block_len && i<N; i++) {
                    for(int j=0; j<block_len && j+jb*block_len<M; j++) {
                        tmp[j] = A[i][j + jb * block_len];
                    }
                    for(int j=0; j<block_len && j+jb*block_len<M; j++) {
                        B[j + jb * block_len][i] = tmp[j];
                    }
                }
            } else {  // A and B not conflict on cache
                for(int i = ib*block_len; i < (ib+1)*block_len && i < N; i++) {
                    for(int j = jb*block_len; j < (jb+1)*block_len && j < M; j++) {
                        B[j][i] = A[i][j];
                    }
                }
            }
        }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

// A simple baseline transpose function, not optimized for the cache.
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N]) {
    int i, j, tmp;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
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
void registerFunctions() {
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
int is_transpose(int M, int N, int A[N][M], int B[M][N]) {
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
