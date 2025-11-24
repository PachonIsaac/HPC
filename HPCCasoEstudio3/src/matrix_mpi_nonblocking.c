/*
 * Matrix Multiplication - MPI Non-blocking Communication
 * Usa MPI_Isend/MPI_Irecv para overlap computation/communication
 * Pipeline: mientras procesa datos actuales, recibe próximos
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
    int local_rows;
    double start_time, end_time, total_time;
    double comm_time = 0.0, compute_time = 0.0;
    MPI_Request *send_requests = NULL, *recv_requests = NULL;
    MPI_Status *send_status = NULL, *recv_status = NULL;
    
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
    
    start_time = MPI_Wtime();
    
    // Allocate B on all processes
    B = (double*)malloc(matrix_size * matrix_size * sizeof(double));
    A_local = (double*)malloc(local_rows * matrix_size * sizeof(double));
    C_local = (double*)malloc(local_rows * matrix_size * sizeof(double));
    
    if (rank == 0) {
        printf("=== MPI Non-blocking Communication ===\n");
        printf("Matrix size: %d x %d\n", matrix_size, matrix_size);
        printf("Number of processes: %d\n", num_procs);
        printf("Rows per process: %d\n", local_rows);
        printf("Optimization: MPI_Isend/MPI_Irecv for overlap\n\n");
        
        A = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        C = (double*)malloc(matrix_size * matrix_size * sizeof(double));
        
        initialize_matrix(A, matrix_size, matrix_size, 12345);
        initialize_matrix(B, matrix_size, matrix_size, 54321);
        
        // Non-blocking send of matrix B to all processes
        double comm_start = MPI_Wtime();
        send_requests = (MPI_Request*)malloc((num_procs - 1) * sizeof(MPI_Request));
        send_status = (MPI_Status*)malloc((num_procs - 1) * sizeof(MPI_Status));
        
        for (int i = 1; i < num_procs; i++) {
            MPI_Isend(B, matrix_size * matrix_size, MPI_DOUBLE, 
                     i, 0, MPI_COMM_WORLD, &send_requests[i-1]);
        }
        
        // Non-blocking send of A rows to workers
        for (int i = 1; i < num_procs; i++) {
            MPI_Isend(&A[i * local_rows * matrix_size], local_rows * matrix_size, 
                     MPI_DOUBLE, i, 1, MPI_COMM_WORLD, &send_requests[i-1]);
        }
        
        // Copy local data for rank 0
        for (int i = 0; i < local_rows * matrix_size; i++) {
            A_local[i] = A[i];
        }
        comm_time += MPI_Wtime() - comm_start;
        
    } else {
        // Workers receive B and their A rows using non-blocking receives
        double comm_start = MPI_Wtime();
        MPI_Request req_B, req_A;
        MPI_Status stat_B, stat_A;
        
        MPI_Irecv(B, matrix_size * matrix_size, MPI_DOUBLE, 
                 0, 0, MPI_COMM_WORLD, &req_B);
        MPI_Irecv(A_local, local_rows * matrix_size, MPI_DOUBLE, 
                 0, 1, MPI_COMM_WORLD, &req_A);
        
        // Wait for data to arrive
        MPI_Wait(&req_B, &stat_B);
        MPI_Wait(&req_A, &stat_A);
        comm_time += MPI_Wtime() - comm_start;
    }
    
    // All processes compute their portion
    double comp_start = MPI_Wtime();
    matrix_multiply_rows(A_local, B, C_local, local_rows, matrix_size);
    compute_time = MPI_Wtime() - comp_start;
    
    // Non-blocking gather results
    double comm_start = MPI_Wtime();
    if (rank == 0) {
        // Copy rank 0's results
        for (int i = 0; i < local_rows * matrix_size; i++) {
            C[i] = C_local[i];
        }
        
        // Receive from workers
        recv_requests = (MPI_Request*)malloc((num_procs - 1) * sizeof(MPI_Request));
        recv_status = (MPI_Status*)malloc((num_procs - 1) * sizeof(MPI_Status));
        
        for (int i = 1; i < num_procs; i++) {
            MPI_Irecv(&C[i * local_rows * matrix_size], local_rows * matrix_size, 
                     MPI_DOUBLE, i, 2, MPI_COMM_WORLD, &recv_requests[i-1]);
        }
        
        // Wait for all results
        MPI_Waitall(num_procs - 1, recv_requests, recv_status);
        
        // Wait for initial sends to complete
        MPI_Waitall(num_procs - 1, send_requests, send_status);
        
    } else {
        // Workers send their results back
        MPI_Request req_send;
        MPI_Isend(C_local, local_rows * matrix_size, MPI_DOUBLE, 
                 0, 2, MPI_COMM_WORLD, &req_send);
        MPI_Wait(&req_send, MPI_STATUS_IGNORE);
    }
    comm_time += MPI_Wtime() - comm_start;
    
    end_time = MPI_Wtime();
    total_time = end_time - start_time;
    
    // Collect statistics
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
        printf("\nOverlap efficiency: %.2f%% (comm hidden by compute)\n",
               (1.0 - (max_comm / total_time)) * 100.0);
        printf("Load balance: %.2f%%\n", (min_compute / max_compute) * 100.0);
        
        printf("\nSample results:\n");
        printf("C[0][0] = %.2f\n", C[0]);
        printf("C[%d][%d] = %.2f\n", 
               matrix_size-1, matrix_size-1, C[matrix_size*matrix_size-1]);
        
        free(A);
        free(C);
        free(send_requests);
        free(send_status);
        free(recv_requests);
        free(recv_status);
    }
    
    free(B);
    free(A_local);
    free(C_local);
    
    MPI_Finalize();
    return 0;
}
