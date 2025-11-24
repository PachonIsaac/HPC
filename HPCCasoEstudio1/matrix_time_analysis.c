#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

// Estructura para pasar datos a cada hilo
typedef struct {
    int **A;
    int **B;
    int **C;
    int size;
    int start_row;
    int end_row;
    int thread_id;
} thread_data_t;

// Función para obtener tiempo de usuario en segundos
double get_user_time() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
}

// Función para obtener tiempo de reloj de pared en segundos
double get_wall_time() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec + time.tv_usec / 1000000.0;
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
    if (matrix == NULL) return NULL;
    
    for (int i = 0; i < size; i++) {
        matrix[i] = (int*)malloc(size * sizeof(int));
        if (matrix[i] == NULL) {
            for (int j = 0; j < i; j++) free(matrix[j]);
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

// Función que ejecuta cada hilo
void* thread_matrix_multiply(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    
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

// Función de multiplicación secuencial
void matrix_multiply_sequential(int **A, int **B, int **C, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            C[i][j] = 0;
            for (int k = 0; k < size; k++) {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Función de multiplicación paralela
void matrix_multiply_parallel(int **A, int **B, int **C, int size, int num_threads) {
    pthread_t *threads = (pthread_t*)malloc(num_threads * sizeof(pthread_t));
    thread_data_t *thread_data = (thread_data_t*)malloc(num_threads * sizeof(thread_data_t));
    
    int rows_per_thread = size / num_threads;
    int remaining_rows = size % num_threads;
    
    int current_row = 0;
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].A = A;
        thread_data[i].B = B;
        thread_data[i].C = C;
        thread_data[i].size = size;
        thread_data[i].thread_id = i;
        thread_data[i].start_row = current_row;
        
        int extra_row = (i < remaining_rows) ? 1 : 0;
        thread_data[i].end_row = current_row + rows_per_thread + extra_row;
        current_row = thread_data[i].end_row;
        
        pthread_create(&threads[i], NULL, thread_matrix_multiply, &thread_data[i]);
    }
    
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    free(threads);
    free(thread_data);
}

void print_usage(char *program_name) {
    printf("Uso: %s <tamaño_matriz> [num_hilos] [semilla_A] [semilla_B]\n", program_name);
    printf("  tamaño_matriz: Tamaño de las matrices cuadradas\n");
    printf("  num_hilos: Número de hilos (por defecto: número de CPUs)\n");
    printf("  semilla_A: Semilla para matriz A\n");
    printf("  semilla_B: Semilla para matriz B\n");
}

int main(int argc, char *argv[]) {
    int size, num_threads;
    int seed_A, seed_B;
    
    if (argc < 2 || argc > 5) {
        print_usage(argv[0]);
        return 1;
    }
    
    size = atoi(argv[1]);
    if (size <= 0) {
        printf("Error: Tamaño debe ser positivo\n");
        return 1;
    }
    
    num_threads = (argc >= 3) ? atoi(argv[2]) : (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0) num_threads = 4;
    
    seed_A = (argc >= 4) ? atoi(argv[3]) : (int)time(NULL);
    seed_B = (argc == 5) ? atoi(argv[4]) : seed_A + 1;
    
    printf("=== Medición de Tiempo de Usuario vs Tiempo de Pared ===\n");
    printf("Tamaño: %dx%d, Hilos: %d\n", size, size, num_threads);
    
    // Allocar matrices
    int **A = allocate_matrix(size);
    int **B = allocate_matrix(size);
    int **C_seq = allocate_matrix(size);
    int **C_par = allocate_matrix(size);
    
    if (!A || !B || !C_seq || !C_par) {
        printf("Error: No se pudo allocar memoria\n");
        return 1;
    }
    
    initialize_matrix(A, size, seed_A);
    initialize_matrix(B, size, seed_B);
    
    // === EJECUCIÓN SECUENCIAL ===
    printf("\n--- SECUENCIAL ---\n");
    double seq_user_start = get_user_time();
    double seq_wall_start = get_wall_time();
    
    matrix_multiply_sequential(A, B, C_seq, size);
    
    double seq_user_end = get_user_time();
    double seq_wall_end = get_wall_time();
    
    double seq_user_time = seq_user_end - seq_user_start;
    double seq_wall_time = seq_wall_end - seq_wall_start;
    
    printf("Tiempo de usuario: %.6f segundos\n", seq_user_time);
    printf("Tiempo de pared: %.6f segundos\n", seq_wall_time);
    printf("GFLOPS (pared): %.6f\n", (2.0 * size * size * size) / (seq_wall_time * 1e9));
    
    // === EJECUCIÓN PARALELA ===
    printf("\n--- PARALELO (%d hilos) ---\n", num_threads);
    double par_user_start = get_user_time();
    double par_wall_start = get_wall_time();
    
    matrix_multiply_parallel(A, B, C_par, size, num_threads);
    
    double par_user_end = get_user_time();
    double par_wall_end = get_wall_time();
    
    double par_user_time = par_user_end - par_user_start;
    double par_wall_time = par_wall_end - par_wall_start;
    
    printf("Tiempo de usuario: %.6f segundos\n", par_user_time);
    printf("Tiempo de pared: %.6f segundos\n", par_wall_time);
    printf("GFLOPS (pared): %.6f\n", (2.0 * size * size * size) / (par_wall_time * 1e9));
    
    // === RESULTADOS ===
    printf("\n=== ANÁLISIS DE SPEEDUP ===\n");
    double speedup_wall = seq_wall_time / par_wall_time;
    double efficiency = (speedup_wall / num_threads) * 100;
    
    printf("Speedup (tiempo de pared): %.2fx\n", speedup_wall);
    printf("Eficiencia: %.2f%%\n", efficiency);
    printf("Ratio tiempo usuario: %.2fx (normal en paralelo)\n", par_user_time / seq_user_time);
    
    // Verificación
    printf("\nVerificando...\n");
    int correct = 1;
    for (int i = 0; i < size && correct; i++) {
        for (int j = 0; j < size && correct; j++) {
            if (C_seq[i][j] != C_par[i][j]) correct = 0;
        }
    }
    printf("%s\n", correct ? "✓ Resultados correctos" : "✗ Error en resultados");
    
    // Liberar memoria
    free_matrix(A, size);
    free_matrix(B, size);
    free_matrix(C_seq, size);
    free_matrix(C_par, size);
    
    return 0;
}