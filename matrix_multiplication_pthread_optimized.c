#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

// Estructura para pasar datos a cada hilo
typedef struct {
    int **A;              // Matriz A
    int **B;              // Matriz B
    int **C;              // Matriz resultado C
    int size;             // Tamaño de la matriz
    int start_row;        // Fila inicial para este hilo
    int end_row;          // Fila final para este hilo
    int thread_id;        // ID del hilo
} thread_data_t;

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
    if (matrix == NULL) {
        return NULL;
    }
    
    for (int i = 0; i < size; i++) {
        matrix[i] = (int*)malloc(size * sizeof(int));
        if (matrix[i] == NULL) {
            // Liberar memoria ya allocada en caso de error
            for (int j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
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

// Función que ejecuta cada hilo - división por filas
void* thread_matrix_multiply(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    
    // Cada hilo procesa un rango de filas
    for (int i = data->start_row; i < data->end_row; i++) {
        for (int j = 0; j < data->size; j++) {
            data->C[i][j] = 0;
            for (int k = 0; k < data->size; k++) {
                data->C[i][j] += data->A[i][k] * data->B[k][j];
            }
        }
    }
    
    pthread_exit(NULL);
}

// Función de multiplicación paralela con pthreads (solo paralela)
double matrix_multiply_parallel_only(int **A, int **B, int **C, int size, int num_threads) {
    pthread_t *threads;
    thread_data_t *thread_data;
    int rows_per_thread, remaining_rows;
    clock_t start, end;
    
    // Allocar memoria para hilos y datos
    threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    thread_data = (thread_data_t*)malloc(num_threads * sizeof(thread_data_t));
    
    if (threads == NULL || thread_data == NULL) {
        printf("Error: No se pudo allocar memoria para hilos\n");
        return -1.0;
    }
    
    // Calcular distribución de trabajo
    rows_per_thread = size / num_threads;
    remaining_rows = size % num_threads;
    
    // Iniciar medición de tiempo
    start = clock();
    
    // Crear hilos
    int current_row = 0;
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].A = A;
        thread_data[i].B = B;
        thread_data[i].C = C;
        thread_data[i].size = size;
        thread_data[i].thread_id = i;
        thread_data[i].start_row = current_row;
        
        // Distribuir filas extras entre los primeros hilos
        int extra_row = (i < remaining_rows) ? 1 : 0;
        thread_data[i].end_row = current_row + rows_per_thread + extra_row;
        
        current_row = thread_data[i].end_row;
        
        // Crear hilo
        int result = pthread_create(&threads[i], NULL, thread_matrix_multiply, &thread_data[i]);
        if (result != 0) {
            printf("Error: No se pudo crear el hilo %d\n", i);
            free(threads);
            free(thread_data);
            return -1.0;
        }
    }
    
    // Esperar a que todos los hilos terminen
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Terminar medición de tiempo
    end = clock();
    
    // Liberar memoria
    free(threads);
    free(thread_data);
    
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

// Función de multiplicación secuencial
double matrix_multiply_sequential_only(int **A, int **B, int **C, int size) {
    clock_t start, end;
    
    start = clock();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            C[i][j] = 0;
            for (int k = 0; k < size; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    
    end = clock();
    
    return ((double)(end - start)) / CLOCKS_PER_SEC;
}

// Función para mostrar ayuda
void print_usage(char *program_name) {
    printf("Uso: %s <tamaño_matriz> [num_hilos] [semilla_A] [semilla_B]\n", program_name);
    printf("  tamaño_matriz: Tamaño de las matrices cuadradas (obligatorio)\n");
    printf("  num_hilos: Número de hilos a usar (opcional, por defecto: número de CPUs)\n");
    printf("  semilla_A: Semilla para generar matriz A (opcional, por defecto: tiempo actual)\n");
    printf("  semilla_B: Semilla para generar matriz B (opcional, por defecto: tiempo actual + 1)\n");
    printf("\nEjemplos:\n");
    printf("  %s 512           # Matriz 512x512, hilos automáticos\n", program_name);
    printf("  %s 1000 4        # Matriz 1000x1000, 4 hilos\n", program_name);
    printf("  %s 512 8 123 456 # Matriz 512x512, 8 hilos, semillas específicas\n", program_name);
}

int main(int argc, char *argv[]) {
    int size, num_threads;
    int seed_A, seed_B;
    double sequential_time, parallel_time, speedup;
    
    // Verificar argumentos de línea de comandos
    if (argc < 2 || argc > 5) {
        print_usage(argv[0]);
        return 1;
    }
    
    // Obtener tamaño de la matriz
    size = atoi(argv[1]);
    if (size <= 0) {
        printf("Error: El tamaño de la matriz debe ser un número positivo.\n");
        return 1;
    }
    
    // Obtener número de hilos
    if (argc >= 3) {
        num_threads = atoi(argv[2]);
        if (num_threads <= 0) {
            printf("Error: El número de hilos debe ser positivo.\n");
            return 1;
        }
    } else {
        // Usar número de CPUs por defecto
        num_threads = (int)sysconf(_SC_NPROCESSORS_ONLN);
        if (num_threads <= 0) num_threads = 4; // Fallback
    }
    
    // Obtener semillas para la generación aleatoria
    if (argc >= 4) {
        seed_A = atoi(argv[3]);
    } else {
        seed_A = (int)time(NULL);
    }
    
    if (argc == 5) {
        seed_B = atoi(argv[4]);
    } else {
        seed_B = seed_A + 1;
    }
    
    printf("=== Multiplicación de Matrices Paralela Optimizada ===\n");
    printf("Tamaño de matrices: %dx%d\n", size, size);
    printf("Número de hilos: %d\n", num_threads);
    printf("Semilla matriz A: %d\n", seed_A);
    printf("Semilla matriz B: %d\n", seed_B);
    printf("Allocando memoria...\n");
    
    // Alocar memoria para las matrices
    int **A = allocate_matrix(size);
    int **B = allocate_matrix(size);
    int **C_sequential = allocate_matrix(size);
    int **C_parallel = allocate_matrix(size);
    
    if (A == NULL || B == NULL || C_sequential == NULL || C_parallel == NULL) {
        printf("Error: No se pudo alocar memoria para las matrices.\n");
        if (A) free_matrix(A, size);
        if (B) free_matrix(B, size);
        if (C_sequential) free_matrix(C_sequential, size);
        if (C_parallel) free_matrix(C_parallel, size);
        return 1;
    }
    
    printf("Inicializando matrices con valores aleatorios...\n");
    
    // Inicializar matrices A y B con valores aleatorios
    initialize_matrix(A, size, seed_A);
    initialize_matrix(B, size, seed_B);
    
    // === EJECUCIÓN SECUENCIAL ===
    printf("\nEjecutando versión secuencial...\n");
    sequential_time = matrix_multiply_sequential_only(A, B, C_sequential, size);
    
    // === EJECUCIÓN PARALELA ===
    printf("Ejecutando versión paralela...\n");
    parallel_time = matrix_multiply_parallel_only(A, B, C_parallel, size, num_threads);
    
    if (parallel_time < 0) {
        printf("Error en ejecución paralela\n");
        return 1;
    }
    
    // Calcular speedup
    speedup = sequential_time / parallel_time;
    double efficiency = (speedup / num_threads) * 100;
    
    printf("\n=== RESULTADOS ===\n");
    printf("Tiempo secuencial: %.6f segundos\n", sequential_time);
    printf("Tiempo paralelo: %.6f segundos\n", parallel_time);
    printf("Speedup: %.2fx\n", speedup);
    printf("Eficiencia: %.2f%% (%d hilos)\n", efficiency, num_threads);
    printf("GFLOPS secuencial: %.6f\n", (2.0 * size * size * size) / (sequential_time * 1e9));
    printf("GFLOPS paralelo: %.6f\n", (2.0 * size * size * size) / (parallel_time * 1e9));
    
    // Verificar que los resultados son correctos
    printf("\nVerificando resultados...\n");
    int verification_passed = 1;
    for (int i = 0; i < size && verification_passed; i++) {
        for (int j = 0; j < size && verification_passed; j++) {
            if (C_sequential[i][j] != C_parallel[i][j]) {
                printf("✗ Error: C_seq[%d][%d]=%d != C_par[%d][%d]=%d\n",
                       i, j, C_sequential[i][j], i, j, C_parallel[i][j]);
                verification_passed = 0;
            }
        }
    }
    
    if (verification_passed) {
        printf("✓ Verificación exitosa: Ambos resultados son idénticos\n");
    }
    
    // Calcular suma de verificación
    long long sum_seq = 0, sum_par = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            sum_seq += C_sequential[i][j];
            sum_par += C_parallel[i][j];
        }
    }
    printf("Suma verificación secuencial: %lld\n", sum_seq);
    printf("Suma verificación paralela: %lld\n", sum_par);
    
    // Liberar memoria
    free_matrix(A, size);
    free_matrix(B, size);
    free_matrix(C_sequential, size);
    free_matrix(C_parallel, size);
    
    return 0;
}
