/*
 * Matrix Multiplication - MPI Broadcast Optimized
 * Optimiza la comunicación: solo Broadcast de B (no scatter/gather)
 * Cada proceso calcula un bloque de filas y retorna solo su porción
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
    double *A_local = NULL, *C_local = NULL;
    int local_rows, start_row;
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
        }
        MPI_Finalize();
        return 1;
    }
    
    local_rows = matrix_size / num_procs;
    start_row = rank * local_rows;
    
    start_time = MPI_Wtime();
    
    // Allocate B on all processes (will be broadcast)
    B = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    
    // Master initializes full matrices
    if (rank == 0) {
        printf("=== MPI Broadcast Optimized ===\n");
        printf("Matrix size: %d x %d\n", matrix_size, matrix_size);
        printf("Number of processes: %d\n", num_procs);
        printf("Rows per process: %d\n", local_rows);
        printf("Optimization: Single Bcast for B, direct row computation\n\n");
        
        A = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        C = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        
        initialize_matrix(A, matrix_size, matrix_size, 12345);
        initialize_matrix(B, matrix_size, matrix_size, 54321);
    }
    
    // Allocate local working buffers
    A_local = (double*)malloc(local_rows * matrix_size * sizeof(double));
    C_local = (double*)malloc(local_rows * matrix_size * sizeof(double));
    
    // Broadcast matrix B to all processes (single communication)
    comm_start = MPI_Wtime();
    MPI_Bcast(B, matrix_size * matrix_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    comm_time += MPI_Wtime() - comm_start;
    
    // Scatter rows of A
    comm_start = MPI_Wtime();
    MPI_Scatter(A, local_rows * matrix_size, MPI_DOUBLE,
                A_local, local_rows * matrix_size, MPI_DOUBLE,
                0, MPI_COMM_WORLD);
    comm_time += MPI_Wtime() - comm_start;
    
    // Computation phase
    double comp_start = MPI_Wtime();
    matrix_multiply_rows(A_local, B, C_local, local_rows, matrix_size);
    compute_time = MPI_Wtime() - comp_start;
    
    // Gather results
    comm_start = MPI_Wtime();
    MPI_Gather(C_local, local_rows * matrix_size, MPI_DOUBLE,
               C, local_rows * matrix_size, MPI_DOUBLE,
               0, MPI_COMM_WORLD);
    comm_time += MPI_Wtime() - comm_start;
    
    end_time = MPI_Wtime();
    total_time = end_time - start_time;
    
    // Collect timing statistics from all processes
    double max_compute, min_compute, avg_compute;
    double max_comm, min_comm, avg_comm;
    
    MPI_Reduce(&compute_time, &max_compute, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&compute_time, &min_compute, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&compute_time, &avg_compute, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comm_time, &max_comm, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comm_time, &min_comm, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&comm_time, &avg_comm, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    
    if (rank == 0) {
        avg_compute /= num_procs;
        avg_comm /= num_procs;
        
        printf("Results:\n");
        printf("Total time: %.6f seconds\n", total_time);
        printf("\nComputation time:\n");
        printf("  Max: %.6f s  Min: %.6f s  Avg: %.6f s\n", 
               max_compute, min_compute, avg_compute);
        printf("Communication time:\n");
        printf("  Max: %.6f s  Min: %.6f s  Avg: %.6f s\n", 
               max_comm, min_comm, avg_comm);
        printf("\nLoad balance: %.2f%% (min/max compute)\n", 
               (min_compute / max_compute) * 100.0);
        printf("Comm overhead: %.2f%% of total time\n", 
               (max_comm / total_time) * 100.0);
        
        printf("\nSample results:\n");
        printf("C[0][0] = %.2f\n", C[0]);
        printf("C[%d][%d] = %.2f\n", 
               matrix_size-1, matrix_size-1, C[matrix_size*matrix_size-1]);
        
        free(A);
        free(C);
    }
    
    free(B);
    free(A_local);
    free(C_local);
    
    MPI_Finalize();
    return 0;
}
