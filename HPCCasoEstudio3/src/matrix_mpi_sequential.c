/*
 * Matrix Multiplication - MPI Sequential (Baseline)
 * Solo el proceso 0 realiza la multiplicación
 * Sirve como baseline para medir overhead de MPI
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

void initialize_matrix(double *matrix, int rows, int cols, int seed) {
    srand(seed);
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = (double)(rand() % 100);
    }
}

void matrix_multiply(double *A, double *B, double *C, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            C[i * size + j] = 0.0;
            for (int k = 0; k < size; k++) {
                C[i * size + j] += A[i * size + k] * B[k * size + j];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int rank, size;
    int matrix_size;
    double *A = NULL, *B = NULL, *C = NULL;
    double start_time, end_time, compute_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc != 2) {
        if (rank == 0) {
            printf("Uso: mpirun -np <procs> %s <matrix_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    matrix_size = atoi(argv[1]);
    
    if (rank == 0) {
        printf("=== MPI Sequential Baseline ===\n");
        printf("Matrix size: %d x %d\n", matrix_size, matrix_size);
        printf("Number of processes: %d\n", size);
        printf("Only rank 0 performs computation\n\n");
        
        // Allocate matrices
        A = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        B = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        C = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        
        // Initialize matrices
        initialize_matrix(A, matrix_size, matrix_size, 12345);
        initialize_matrix(B, matrix_size, matrix_size, 54321);
        
        // Start timing
        start_time = MPI_Wtime();
        
        // Perform multiplication (only rank 0)
        matrix_multiply(A, B, C, matrix_size);
        
        // End timing
        end_time = MPI_Wtime();
        compute_time = end_time - start_time;
        
        printf("Computation time: %.6f seconds\n", compute_time);
        printf("Sample result C[0][0] = %.2f\n", C[0]);
        printf("Sample result C[%d][%d] = %.2f\n", 
               matrix_size-1, matrix_size-1, C[matrix_size*matrix_size-1]);
        
        // Free memory
        free(A);
        free(B);
        free(C);
    }
    
    MPI_Finalize();
    return 0;
}
