/*
 * Matrix Multiplication - MPI Row-wise Distribution
 * Master distribuye filas de A a workers
 * Cada worker calcula su porción de C
 * Usa MPI_Scatter y MPI_Gather
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

void matrix_multiply_rows(double *A_local, double *B, double *C_local, 
                          int local_rows, int size) {
    for (int i = 0; i < local_rows; i++) {
        for (int j = 0; j < size; j++) {
            C_local[i * size + j] = 0.0;
            for (int k = 0; k < size; k++) {
                C_local[i * size + j] += A_local[i * size + k] * B[k * size + j];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int rank, num_procs;
    int matrix_size;
    double *A = NULL, *B = NULL, *C = NULL;
    double *A_local = NULL, *B_local = NULL, *C_local = NULL;
    int local_rows;
    double start_time, end_time, total_time;
    double comm_start, comm_time = 0.0, compute_time = 0.0;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    if (argc != 2) {
        if (rank == 0) {
            printf("Uso: mpirun -np <procs> %s <matrix_size>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    matrix_size = atoi(argv[1]);
    
    if (matrix_size % num_procs != 0) {
        if (rank == 0) {
            printf("Error: Matrix size must be divisible by number of processes\n");
            printf("Matrix size: %d, Processes: %d\n", matrix_size, num_procs);
        }
        MPI_Finalize();
        return 1;
    }
    
    local_rows = matrix_size / num_procs;
    
    // Start total timing
    start_time = MPI_Wtime();
    
    // Master process initializes matrices
    if (rank == 0) {
        printf("=== MPI Row-wise Distribution ===\n");
        printf("Matrix size: %d x %d\n", matrix_size, matrix_size);
        printf("Number of processes: %d\n", num_procs);
        printf("Rows per process: %d\n\n", local_rows);
        
        A = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        B = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        C = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        
        initialize_matrix(A, matrix_size, matrix_size, 12345);
        initialize_matrix(B, matrix_size, matrix_size, 54321);
    }
    
    // All processes allocate local buffers
    A_local = (double*)malloc(local_rows * matrix_size * sizeof(double));
    B_local = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    C_local = (double*)malloc(local_rows * matrix_size * sizeof(double));
    
    // Distribute rows of A using Scatter
    comm_start = MPI_Wtime();
    MPI_Scatter(A, local_rows * matrix_size, MPI_DOUBLE,
                A_local, local_rows * matrix_size, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    comm_time += MPI_Wtime() - comm_start;
    
    // Broadcast entire matrix B to all processes
    comm_start = MPI_Wtime();
    if (rank == 0) {
        for (int i = 0; i < matrix_size * matrix_size; i++) {
            B_local[i] = B[i];
        }
    }
    MPI_Bcast(B_local, matrix_size * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    comm_time += MPI_Wtime() - comm_start;
    
    // Local computation
    double comp_start = MPI_Wtime();
    matrix_multiply_rows(A_local, B_local, C_local, local_rows, matrix_size);
    compute_time = MPI_Wtime() - comp_start;
    
    // Gather results back to master
    comm_start = MPI_Wtime();
    MPI_Gather(C_local, local_rows * matrix_size, MPI_DOUBLE,
               C, local_rows * matrix_size, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
    comm_time += MPI_Wtime() - comm_start;
    
    end_time = MPI_Wtime();
    total_time = end_time - start_time;
    
    // Master prints results
    if (rank == 0) {
        printf("Results:\n");
        printf("Total time: %.6f seconds\n", total_time);
        printf("Computation time (rank 0): %.6f seconds\n", compute_time);
        printf("Communication time (rank 0): %.6f seconds\n", comm_time);
        printf("Speedup potential: %.2fx (if comm was zero)\n", 
               total_time / compute_time);
        printf("\nSample results:\n");
        printf("C[0][0] = %.2f\n", C[0]);
        printf("C[%d][%d] = %.2f\n", 
               matrix_size-1, matrix_size-1, C[matrix_size*matrix_size-1]);
        
        free(A);
        free(B);
        free(C);
    }
    
    free(A_local);
    free(B_local);
    free(C_local);
    
    MPI_Finalize();
    return 0;
}
