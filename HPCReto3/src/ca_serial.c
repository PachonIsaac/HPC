/*
 * Cellular Automaton - Traffic Flow Simulation (Serial Version)
 * 
 * Model: 1D circular road with periodic boundary conditions
 * Rule: Car moves forward if next cell is empty, otherwise stays
 * Metric: velocity = (cars moved) / (total cars)
 * 
 * Compilation:
 *   gcc -O2 -o ca_serial ca_serial.c -lm
 * 
 * Usage:
 *   ./ca_serial <N> <T> <density> [seed]
 *   N       : Number of cells (road length)
 *   T       : Number of timesteps
 *   density : Initial car density [0.0, 1.0]
 *   seed    : Random seed (optional, default: 42)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Function prototypes
void initialize_road(int *road, int N, double density, unsigned int seed);
int update_timestep(int *old_road, int *new_road, int N);
double calculate_velocity(int moved_count, int total_cars);
int count_cars(int *road, int N);
unsigned long checksum(int *road, int N);
void print_road(int *road, int N);
void print_statistics(double *velocities, int T);

int main(int argc, char *argv[]) {
    // Parse command line arguments
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <N> <T> <density> [seed]\n", argv[0]);
        fprintf(stderr, "  N       : Number of cells (road length)\n");
        fprintf(stderr, "  T       : Number of timesteps\n");
        fprintf(stderr, "  density : Initial car density [0.0, 1.0]\n");
        fprintf(stderr, "  seed    : Random seed (optional, default: 42)\n");
        return 1;
    }

    int N = atoi(argv[1]);           // Number of cells
    int T = atoi(argv[2]);           // Number of timesteps
    double density = atof(argv[3]);  // Initial density
    unsigned int seed = (argc >= 5) ? atoi(argv[4]) : 42;

    // Validation
    if (N <= 0 || T <= 0) {
        fprintf(stderr, "Error: N and T must be positive integers\n");
        return 1;
    }
    if (density < 0.0 || density > 1.0) {
        fprintf(stderr, "Error: density must be in [0.0, 1.0]\n");
        return 1;
    }

    // Allocate memory
    int *old_road = (int *)calloc(N, sizeof(int));
    int *new_road = (int *)calloc(N, sizeof(int));
    double *velocities = (double *)malloc(T * sizeof(double));
    
    if (!old_road || !new_road || !velocities) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(old_road);
        free(new_road);
        free(velocities);
        return 1;
    }

    // Initialize road with random cars
    initialize_road(old_road, N, density, seed);
    int total_cars = count_cars(old_road, N);

    printf("Cellular Automaton Traffic Simulation (Serial)\n");
    printf("================================================\n");
    printf("Parameters:\n");
    printf("  Road length (N):    %d cells\n", N);
    printf("  Timesteps (T):      %d\n", T);
    printf("  Initial density:    %.3f\n", density);
    printf("  Total cars:         %d\n", total_cars);
    printf("  Random seed:        %u\n", seed);
    printf("  Initial checksum:   %lu\n", checksum(old_road, N));
    printf("================================================\n\n");

    // Print initial state for small roads
    if (N <= 100) {
        printf("Initial state:\n");
        print_road(old_road, N);
        printf("\n");
    }

    // Start timing
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    // Main simulation loop
    for (int t = 0; t < T; t++) {
        // Update road state and count moves
        int moved_count = update_timestep(old_road, new_road, N);
        
        // Calculate velocity
        velocities[t] = calculate_velocity(moved_count, total_cars);
        
        // Swap buffers (old becomes new)
        int *temp = old_road;
        old_road = new_road;
        new_road = temp;
        
        // Periodic output (every 10% of timesteps or for small T)
        if (T <= 20 || (t + 1) % (T / 10) == 0) {
            printf("t=%6d: velocity=%.4f, moved=%d/%d\n", 
                   t + 1, velocities[t], moved_count, total_cars);
        }
    }

    // End timing
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

    // Print final state for small roads
    if (N <= 100) {
        printf("\nFinal state:\n");
        print_road(old_road, N);
        printf("\n");
    }

    // Verify conservation of cars
    int final_cars = count_cars(old_road, N);
    printf("\n================================================\n");
    printf("Verification:\n");
    printf("  Initial cars:       %d\n", total_cars);
    printf("  Final cars:         %d\n", final_cars);
    printf("  Conservation:       %s\n", (total_cars == final_cars) ? "PASS" : "FAIL");
    printf("  Final checksum:     %lu\n", checksum(old_road, N));
    printf("================================================\n\n");

    // Print statistics
    print_statistics(velocities, T);

    // Print timing
    printf("\n================================================\n");
    printf("Performance:\n");
    printf("  Total time:         %.6f seconds\n", elapsed);
    printf("  Time per timestep:  %.6f seconds\n", elapsed / T);
    printf("  Cell updates/sec:   %.2e\n", (double)(N) * T / elapsed);
    printf("================================================\n");

    // Cleanup
    free(old_road);
    free(new_road);
    free(velocities);

    return 0;
}

/**
 * Initialize road with random cars based on density
 */
void initialize_road(int *road, int N, double density, unsigned int seed) {
    srand(seed);
    for (int i = 0; i < N; i++) {
        road[i] = (rand() / (double)RAND_MAX < density) ? 1 : 0;
    }
}

/**
 * Update road for one timestep using CA rules
 * Returns number of cars that moved
 */
int update_timestep(int *old_road, int *new_road, int N) {
    int moved_count = 0;
    
    // Phase 1: Determine new state for each cell
    for (int i = 0; i < N; i++) {
        int left  = old_road[(i - 1 + N) % N];  // Periodic boundary
        int curr  = old_road[i];
        int right = old_road[(i + 1) % N];      // Periodic boundary
        
        // Apply CA rule (see ANALYSIS.md for truth tables)
        if (curr == 1 && right == 1) {
            // Car is blocked, stays in place
            new_road[i] = 1;
        } else if (left == 1 && curr == 0) {
            // Car from left moves into this cell
            new_road[i] = 1;
        } else {
            // Cell is empty or car moved out
            new_road[i] = 0;
        }
    }
    
    // Phase 2: Count how many cars moved
    for (int i = 0; i < N; i++) {
        if (old_road[i] == 1 && new_road[i] == 0) {
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
        return 0.0;  // No cars, velocity undefined (return 0)
    }
    return (double)moved_count / total_cars;
}

/**
 * Count total number of cars on road
 */
int count_cars(int *road, int N) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        count += road[i];
    }
    return count;
}

/**
 * Calculate checksum for verification
 * Formula: sum(i * road[i]) mod large_prime
 */
unsigned long checksum(int *road, int N) {
    unsigned long sum = 0;
    const unsigned long PRIME = 1000000007UL;
    
    for (int i = 0; i < N; i++) {
        if (road[i] == 1) {
            sum = (sum + i) % PRIME;
        }
    }
    return sum;
}

/**
 * Print road state (for small N)
 */
void print_road(int *road, int N) {
    printf("  ");
    for (int i = 0; i < N; i++) {
        printf("%c", road[i] ? 'X' : '.');
        if ((i + 1) % 50 == 0 && i + 1 < N) {
            printf("\n  ");
        }
    }
    printf("\n");
}

/**
 * Print velocity statistics
 */
void print_statistics(double *velocities, int T) {
    // Calculate mean
    double sum = 0.0;
    for (int t = 0; t < T; t++) {
        sum += velocities[t];
    }
    double mean = sum / T;
    
    // Calculate standard deviation
    double sum_sq = 0.0;
    for (int t = 0; t < T; t++) {
        double diff = velocities[t] - mean;
        sum_sq += diff * diff;
    }
    double std_dev = sqrt(sum_sq / T);
    
    // Find min and max
    double min_vel = velocities[0];
    double max_vel = velocities[0];
    for (int t = 1; t < T; t++) {
        if (velocities[t] < min_vel) min_vel = velocities[t];
        if (velocities[t] > max_vel) max_vel = velocities[t];
    }
    
    printf("================================================\n");
    printf("Velocity Statistics:\n");
    printf("  Mean:               %.6f\n", mean);
    printf("  Std Dev:            %.6f\n", std_dev);
    printf("  Min:                %.6f\n", min_vel);
    printf("  Max:                %.6f\n", max_vel);
    printf("  Coefficient of Var: %.2f%%\n", (std_dev / mean) * 100);
    printf("================================================\n");
}
