#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>

// Función para obtener tiempo real (wall time) en segundos
double get_wall_time() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

#define BLOCK_SIZE 32

// Función para obtener tiempo de usuario en segundos
double get_user_time() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
}

// Función para inicializar una matriz con valores aleatorios
void initialize_matrix(int **matrix, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
}

// Función para allocar memoria para una matriz cuadrada
int** allocate_matrix(int size) {
    int **matrix = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        matrix[i] = (int*)malloc(size * sizeof(int));
    }
    return matrix;
}

// Función para liberar la memoria de una matriz
void free_matrix(int **matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Multiplicación de matrices con blocking (sin OpenMP)
void matrix_multiply_blocking_seq(int **A, int **B, int **C, int size) {
    int i, j, k, ii, jj, kk;
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            C[i][j] = 0;
        }
    }
    for (ii = 0; ii < size; ii += BLOCK_SIZE) {
        for (jj = 0; jj < size; jj += BLOCK_SIZE) {
            for (kk = 0; kk < size; kk += BLOCK_SIZE) {
                for (i = ii; i < ii + BLOCK_SIZE && i < size; i++) {
                    for (j = jj; j < jj + BLOCK_SIZE && j < size; j++) {
                        int sum = C[i][j];
                        for (k = kk; k < kk + BLOCK_SIZE && k < size; k++) {
                            sum += A[i][k] * B[k][j];
                        }
                        C[i][j] = sum;
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int size;
    int seed_A, seed_B;
    double start_time, end_time, wall_start, wall_end;
    double cpu_time_used, wall_time_used;

    if (argc < 2 || argc > 4) {
        printf("Uso: %s <tamaño_matriz> [semilla_A] [semilla_B]\n", argv[0]);
        return 1;
    }

    size = atoi(argv[1]);
    seed_A = (argc >= 3) ? atoi(argv[2]) : (int)time(NULL);
    seed_B = (argc == 4) ? atoi(argv[3]) : seed_A + 1;

    int **A = allocate_matrix(size);
    int **B = allocate_matrix(size);
    int **C = allocate_matrix(size);

    initialize_matrix(A, size, seed_A);
    initialize_matrix(B, size, seed_B);

    start_time = get_user_time();
    wall_start = get_wall_time();
    matrix_multiply_blocking_seq(A, B, C, size);
    wall_end = get_wall_time();
    end_time = get_user_time();

    cpu_time_used = end_time - start_time;
    wall_time_used = wall_end - wall_start;
    printf("Tiempo de usuario: %.6f segundos\n", cpu_time_used);
    printf("Tiempo real (wall time): %.6f segundos\n", wall_time_used);

    // Calcular suma de verificación
    long long sum = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            sum += C[i][j];
        }
    }
    printf("Suma de verificación de la matriz resultado: %lld\n", sum);

    free_matrix(A, size);
    free_matrix(B, size);
    free_matrix(C, size);
    return 0;
}
