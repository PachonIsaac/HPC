/*
 * Cellular Automaton - Traffic Flow Simulation (MPI Non-Blocking Version)
 * 
 * Model: 1D circular road with periodic boundary conditions
 * Parallelization: Domain decomposition with non-blocking halo exchange
 * Communication: MPI_Isend/Irecv with computation-communication overlap
 * 
 * Optimization: Updates interior cells while ghost exchange is in progress
 * 
 * Compilation:
 *   mpicc -O2 -o ca_mpi_nonblocking ca_mpi_nonblocking.c -lm
 * 
 * Usage:
 *   mpirun -np <P> ./ca_mpi_nonblocking <N> <T> <density> [seed]
 *   P       : Number of MPI processes
 *   N       : Number of cells (must be divisible by P)
 *   T       : Number of timesteps
 *   density : Initial car density [0.0, 1.0]
 *   seed    : Random seed (optional, default: 42)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

// MPI tags for communication
#define TAG_LEFT  100
#define TAG_RIGHT 101

// Function prototypes
void initialize_local_road(int *local_road, int local_N, double density, unsigned int seed);
void start_ghost_exchange(int *local_old, int local_N, int *ghost_left, int *ghost_right,
                          int left_rank, int right_rank, MPI_Request requests[4]);
void complete_ghost_exchange(MPI_Request requests[4], int *ghost_left, int *ghost_right);
int update_interior_cells(int *local_old, int *local_new, int local_N);
int update_boundary_cells(int *local_old, int *local_new, int local_N, 
                          int ghost_left, int ghost_right);
double calculate_velocity(int moved_count, int total_cars);
int count_local_cars(int *local_road, int local_N);

int main(int argc, char *argv[]) {
    int rank, size;
    double start_time, end_time;
    
    // Initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Parse command line arguments
    if (argc < 4) {
        if (rank == 0) {
            fprintf(stderr, "Usage: mpirun -np <P> %s <N> <T> <density> [seed]\n", argv[0]);
            fprintf(stderr, "  N       : Number of cells (must be divisible by P=%d)\n", size);
            fprintf(stderr, "  T       : Number of timesteps\n");
            fprintf(stderr, "  density : Initial car density [0.0, 1.0]\n");
            fprintf(stderr, "  seed    : Random seed (optional, default: 42)\n");
        }
        MPI_Finalize();
        return 1;
    }

    int N = atoi(argv[1]);
    int T = atoi(argv[2]);
    double density = atof(argv[3]);
    unsigned int base_seed = (argc >= 5) ? atoi(argv[4]) : 42;

    // Validation
    if (N <= 0 || T <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: N and T must be positive integers\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    if (density < 0.0 || density > 1.0) {
        if (rank == 0) {
            fprintf(stderr, "Error: density must be in [0.0, 1.0]\n");
        }
        MPI_Finalize();
        return 1;
    }
    
    // Check if N is divisible by P
    if (N % size != 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: N=%d must be divisible by P=%d\n", N, size);
            fprintf(stderr, "Suggestion: Use N=%d (next multiple)\n", 
                    ((N / size) + 1) * size);
        }
        MPI_Finalize();
        return 1;
    }

    // Calculate local domain size
    int local_N = N / size;
    
    // Calculate neighbor ranks (circular topology)
    int left_rank = (rank - 1 + size) % size;
    int right_rank = (rank + 1) % size;

    // Allocate local arrays
    int *local_old = (int *)calloc(local_N, sizeof(int));
    int *local_new = (int *)calloc(local_N, sizeof(int));
    double *velocities = NULL;
    
    if (!local_old || !local_new) {
        fprintf(stderr, "Rank %d: Memory allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Only rank 0 stores global velocities
    if (rank == 0) {
        velocities = (double *)malloc(T * sizeof(double));
        if (!velocities) {
            fprintf(stderr, "Rank 0: Memory allocation failed for velocities\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    // Initialize local road (each process with different seed for randomness)
    initialize_local_road(local_old, local_N, density, base_seed + rank);
    
    // Count initial cars (global)
    int local_cars = count_local_cars(local_old, local_N);
    int total_cars = 0;
    MPI_Reduce(&local_cars, &total_cars, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Print header (rank 0 only)
    if (rank == 0) {
        printf("Cellular Automaton Traffic Simulation (MPI Non-Blocking)\n");
        printf("========================================================\n");
        printf("Parameters:\n");
        printf("  Road length (N):      %d cells\n", N);
        printf("  Timesteps (T):        %d\n", T);
        printf("  Initial density:      %.3f\n", density);
        printf("  Total cars:           %d\n", total_cars);
        printf("  MPI processes (P):    %d\n", size);
        printf("  Local cells per proc: %d\n", local_N);
        printf("  Base random seed:     %u\n", base_seed);
        printf("  Optimization:         Communication-computation overlap\n");
        printf("========================================================\n\n");
    }

    // Barrier to synchronize before timing
    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    // Ghost cells for halo exchange
    int ghost_left = 0;
    int ghost_right = 0;
    
    // Requests for non-blocking communication
    MPI_Request requests[4];

    // Main simulation loop with overlap optimization
    for (int t = 0; t < T; t++) {
        // Step 1: Start non-blocking ghost exchange
        start_ghost_exchange(local_old, local_N, &ghost_left, &ghost_right,
                           left_rank, right_rank, requests);
        
        // Step 2: Update INTERIOR cells while communication is in progress
        // (interior cells don't need ghost data)
        int moved_interior = update_interior_cells(local_old, local_new, local_N);
        
        // Step 3: Wait for ghost exchange to complete
        complete_ghost_exchange(requests, &ghost_left, &ghost_right);
        
        // Step 4: Update BOUNDARY cells (now we have ghost data)
        int moved_boundary = update_boundary_cells(local_old, local_new, local_N,
                                                   ghost_left, ghost_right);
        
        // Step 5: Sum local moves
        int local_moved = moved_interior + moved_boundary;
        
        // Step 6: Global reduction to sum moved cars
        int global_moved = 0;
        MPI_Reduce(&local_moved, &global_moved, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        
        // Step 7: Calculate and store velocity (rank 0 only)
        if (rank == 0) {
            velocities[t] = calculate_velocity(global_moved, total_cars);
            
            // Periodic output
            if (T <= 20 || (t + 1) % (T / 10) == 0) {
                printf("t=%6d: velocity=%.4f, moved=%d/%d\n", 
                       t + 1, velocities[t], global_moved, total_cars);
            }
        }
        
        // Step 8: Swap buffers (old becomes new)
        int *temp = local_old;
        local_old = local_new;
        local_new = temp;
    }

    // End timing
    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    double elapsed = end_time - start_time;

    // Verify conservation (final car count)
    local_cars = count_local_cars(local_old, local_N);
    int final_cars = 0;
    MPI_Reduce(&local_cars, &final_cars, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Print results (rank 0 only)
    if (rank == 0) {
        printf("\n========================================================\n");
        printf("Verification:\n");
        printf("  Initial cars:         %d\n", total_cars);
        printf("  Final cars:           %d\n", final_cars);
        printf("  Conservation:         %s\n", (total_cars == final_cars) ? "PASS" : "FAIL");
        printf("========================================================\n\n");

        // Calculate velocity statistics
        double sum = 0.0;
        for (int t = 0; t < T; t++) {
            sum += velocities[t];
        }
        double mean = sum / T;
        
        double sum_sq = 0.0;
        for (int t = 0; t < T; t++) {
            double diff = velocities[t] - mean;
            sum_sq += diff * diff;
        }
        double std_dev = sqrt(sum_sq / T);
        
        double min_vel = velocities[0];
        double max_vel = velocities[0];
        for (int t = 1; t < T; t++) {
            if (velocities[t] < min_vel) min_vel = velocities[t];
            if (velocities[t] > max_vel) max_vel = velocities[t];
        }
        
        printf("========================================================\n");
        printf("Velocity Statistics:\n");
        printf("  Mean:                 %.6f\n", mean);
        printf("  Std Dev:              %.6f\n", std_dev);
        printf("  Min:                  %.6f\n", min_vel);
        printf("  Max:                  %.6f\n", max_vel);
        printf("  Coefficient of Var:   %.2f%%\n", (std_dev / mean) * 100);
        printf("========================================================\n\n");

        printf("========================================================\n");
        printf("Performance:\n");
        printf("  Total time:           %.6f seconds\n", elapsed);
        printf("  Time per timestep:    %.6f seconds\n", elapsed / T);
        printf("  Cell updates/sec:     %.2e\n", (double)N * T / elapsed);
        printf("========================================================\n");
    }

    // Cleanup
    free(local_old);
    free(local_new);
    if (rank == 0) {
        free(velocities);
    }

    MPI_Finalize();
    return 0;
}

/**
 * Initialize local road segment with random cars
 */
void initialize_local_road(int *local_road, int local_N, double density, unsigned int seed) {
    srand(seed);
    for (int i = 0; i < local_N; i++) {
        local_road[i] = (rand() / (double)RAND_MAX < density) ? 1 : 0;
    }
}

/**
 * Start non-blocking ghost cell exchange
 * Posts receives first, then sends (avoids buffer issues)
 */
void start_ghost_exchange(int *local_old, int local_N, int *ghost_left, int *ghost_right,
                          int left_rank, int right_rank, MPI_Request requests[4]) {
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    // Special case: single process (no communication needed)
    if (size == 1) {
        *ghost_left = local_old[local_N - 1];  // Wrap around
        *ghost_right = local_old[0];
        // Set dummy requests
        for (int i = 0; i < 4; i++) {
            requests[i] = MPI_REQUEST_NULL;
        }
        return;
    }
    
    // Post non-blocking receives FIRST (important for performance)
    MPI_Irecv(ghost_left, 1, MPI_INT, left_rank, TAG_RIGHT, 
              MPI_COMM_WORLD, &requests[0]);
    MPI_Irecv(ghost_right, 1, MPI_INT, right_rank, TAG_LEFT, 
              MPI_COMM_WORLD, &requests[1]);
    
    // Then post non-blocking sends
    MPI_Isend(&local_old[local_N - 1], 1, MPI_INT, right_rank, TAG_RIGHT, 
              MPI_COMM_WORLD, &requests[2]);
    MPI_Isend(&local_old[0], 1, MPI_INT, left_rank, TAG_LEFT, 
              MPI_COMM_WORLD, &requests[3]);
}

/**
 * Complete ghost cell exchange
 * Waits for all non-blocking operations to finish
 */
void complete_ghost_exchange(MPI_Request requests[4], int *ghost_left, int *ghost_right) {
    MPI_Status statuses[4];
    MPI_Waitall(4, requests, statuses);
}

/**
 * Update INTERIOR cells (indices 1 to local_N-2)
 * These cells don't need ghost data, so can be computed during communication
 * Returns number of cars that moved in the interior
 */
int update_interior_cells(int *local_old, int *local_new, int local_N) {
    int moved_count = 0;
    
    // Special case: very small domains (local_N <= 2)
    if (local_N <= 2) {
        return 0;  // No interior cells, all are boundaries
    }
    
    // Update interior cells: [1, local_N-2]
    for (int i = 1; i < local_N - 1; i++) {
        int left = local_old[i - 1];
        int curr = local_old[i];
        int right = local_old[i + 1];
        
        // Apply CA rule
        if (curr == 1 && right == 1) {
            local_new[i] = 1;  // Blocked
        } else if (left == 1 && curr == 0) {
            local_new[i] = 1;  // Car enters
        } else {
            local_new[i] = 0;  // Empty or car moved out
        }
    }
    
    // Count moves in interior
    for (int i = 1; i < local_N - 1; i++) {
        if (local_old[i] == 1 && local_new[i] == 0) {
            moved_count++;
        }
    }
    
    return moved_count;
}

/**
 * Update BOUNDARY cells (indices 0 and local_N-1)
 * These cells need ghost data from neighbors
 * Returns number of cars that moved at boundaries
 */
int update_boundary_cells(int *local_old, int *local_new, int local_N,
                         int ghost_left, int ghost_right) {
    int moved_count = 0;
    
    // Update left boundary cell (index 0)
    {
        int left = ghost_left;
        int curr = local_old[0];
        int right = (local_N > 1) ? local_old[1] : ghost_right;
        
        if (curr == 1 && right == 1) {
            local_new[0] = 1;
        } else if (left == 1 && curr == 0) {
            local_new[0] = 1;
        } else {
            local_new[0] = 0;
        }
        
        if (local_old[0] == 1 && local_new[0] == 0) {
            moved_count++;
        }
    }
    
    // Update right boundary cell (index local_N-1)
    if (local_N > 1) {
        int left = local_old[local_N - 2];
        int curr = local_old[local_N - 1];
        int right = ghost_right;
        
        if (curr == 1 && right == 1) {
            local_new[local_N - 1] = 1;
        } else if (left == 1 && curr == 0) {
            local_new[local_N - 1] = 1;
        } else {
            local_new[local_N - 1] = 0;
        }
        
        if (local_old[local_N - 1] == 1 && local_new[local_N - 1] == 0) {
            moved_count++;
        }
    }
    
    return moved_count;
}

/**
 * Calculate velocity metric
 */
double calculate_velocity(int moved_count, int total_cars) {
    if (total_cars == 0) {
        return 0.0;
    }
    return (double)moved_count / total_cars;
}

/**
 * Count total number of cars in local road segment
 */
int count_local_cars(int *local_road, int local_N) {
    int count = 0;
    for (int i = 0; i < local_N; i++) {
        count += local_road[i];
    }
    return count;
}
