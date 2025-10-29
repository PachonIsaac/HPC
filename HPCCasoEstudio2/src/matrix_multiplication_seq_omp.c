#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <omp.h>

// Función para obtener tiempo real (wall time) en segundos
double get_wall_time() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

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
            matrix[i][j] = rand() % 100; // Valores aleatorios entre 0 y 99
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

// Versión secuencial paralelizada con OpenMP
void matrix_multiply_seq_omp(int **A, int **B, int **C, int size) {
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            C[i][j] = 0;
            for (int k = 0; k < size; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

void print_usage(char *program_name) {
    printf("Uso: %s <tamaño_matriz> [semilla_A] [semilla_B]\n", program_name);
    printf("  tamaño_matriz: Tamaño de las matrices cuadradas (obligatorio)\n");
    printf("  semilla_A: Semilla para generar matriz A (opcional, por defecto: tiempo actual)\n");
    printf("  semilla_B: Semilla para generar matriz B (opcional, por defecto: tiempo actual + 1)\n");
    printf("\nEjemplo: %s 512 123 456\n", program_name);
}

int main(int argc, char *argv[]) {
    int size;
    int seed_A, seed_B;
    double start_time, end_time, wall_start, wall_end;
    double cpu_time_used, wall_time_used;

    if (argc < 2 || argc > 4) {
        print_usage(argv[0]);
        return 1;
    }

    size = atoi(argv[1]);
    if (size <= 0) {
        printf("Error: El tamaño de la matriz debe ser un número positivo.\n");
        return 1;
    }

    if (argc >= 3) {
        seed_A = atoi(argv[2]);
    } else {
        seed_A = (int)time(NULL);
    }

    if (argc == 4) {
        seed_B = atoi(argv[3]);
    } else {
        seed_B = seed_A + 1;
    }

    int **A = allocate_matrix(size);
    int **B = allocate_matrix(size);
    int **C = allocate_matrix(size);

    if (A == NULL || B == NULL || C == NULL) {
        printf("Error: No se pudo alocar memoria para las matrices.\n");
        return 1;
    }

    initialize_matrix(A, size, seed_A);
    initialize_matrix(B, size, seed_B);

    start_time = get_user_time();
    wall_start = get_wall_time();
    matrix_multiply_seq_omp(A, B, C, size);
    wall_end = get_wall_time();
    end_time = get_user_time();

    cpu_time_used = end_time - start_time;
    wall_time_used = wall_end - wall_start;
    printf("Tiempo de usuario: %.6f segundos\n", cpu_time_used);
    printf("Tiempo real (wall time): %.6f segundos\n", wall_time_used);

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
