#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <signal.h>
#include <errno.h>

// Estructura para datos compartidos entre procesos
typedef struct {
    int size;           // Tamaño de la matriz
    int num_processes;  // Número de procesos
    int process_id;     // ID del proceso
    int start_row;      // Fila inicial para este proceso
    int end_row;        // Fila final para este proceso
} process_data_t;

// Función para obtener tiempo de usuario en segundos
double get_user_time() {
    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
}

// Función para obtener tiempo de reloj de pared
double get_wall_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

// Función para inicializar una matriz con valores aleatorios
void initialize_matrix(int *matrix, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size * size; i++) {
        matrix[i] = rand() % 100; // Valores aleatorios entre 0 y 99
    }
}

// Función para allocar memoria compartida
void* allocate_shared_memory(size_t size) {
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, 
                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }
    return ptr;
}

// Función para liberar memoria compartida
void free_shared_memory(void *ptr, size_t size) {
    if (munmap(ptr, size) == -1) {
        perror("munmap");
    }
}

// Función de multiplicación de matrices secuencial
double matrix_multiply_sequential(int *A, int *B, int *C, int size) {
    double start_time = get_wall_time();
    double start_user = get_user_time();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            C[i * size + j] = 0;
            for (int k = 0; k < size; k++) {
                C[i * size + j] += A[i * size + k] * B[k * size + j];
            }
        }
    }
    
    double end_time = get_wall_time();
    double end_user = get_user_time();
    
    printf("Tiempo secuencial - Reloj: %.6f s, Usuario: %.6f s\n", 
           end_time - start_time, end_user - start_user);
    
    return end_time - start_time;
}

// Función que ejecuta cada proceso hijo
int process_matrix_multiply(int *A, int *B, int *C, process_data_t *data) {
    int size = data->size;
    int start_row = data->start_row;
    int end_row = data->end_row;
    int process_id = data->process_id;
    
    printf("Proceso %d (PID: %d): procesando filas %d a %d\n", 
           process_id, getpid(), start_row, end_row - 1);
    
    // Cada proceso calcula su porción de filas
    for (int i = start_row; i < end_row; i++) {
        for (int j = 0; j < size; j++) {
            C[i * size + j] = 0;
            for (int k = 0; k < size; k++) {
                C[i * size + j] += A[i * size + k] * B[k * size + j];
            }
        }
    }
    
    printf("Proceso %d completado\n", process_id);
    return 0;
}

// Función de multiplicación paralela con procesos
double matrix_multiply_parallel(int *A, int *B, int *C, int size, int num_processes) {
    pid_t *pids;
    process_data_t *process_data;
    int rows_per_process, remaining_rows;
    double start_time, end_time;
    
    // Allocar memoria y datos de procesos
    pids = malloc(num_processes * sizeof(pid_t));
    process_data = allocate_shared_memory(num_processes * sizeof(process_data_t));
    
    if (pids == NULL || process_data == NULL) {
        printf("Error: No se pudo allocar memoria para procesos\n");
        return -1.0;
    }
    
    // Calcular distribución de trabajo
    rows_per_process = size / num_processes;
    remaining_rows = size % num_processes;
    
    printf("Distribución: %d filas por proceso, %d filas extras\n", 
           rows_per_process, remaining_rows);
    
    // Iniciar medición de tiempo
    start_time = get_wall_time();
    
    // Crear procesos hijos
    int current_row = 0;
    for (int i = 0; i < num_processes; i++) {
        process_data[i].size = size;
        process_data[i].num_processes = num_processes;
        process_data[i].process_id = i;
        process_data[i].start_row = current_row;
        
        // Distribuir filas extras entre los primeros procesos
        int extra_row = (i < remaining_rows) ? 1 : 0;
        process_data[i].end_row = current_row + rows_per_process + extra_row;
        
        current_row = process_data[i].end_row;
        
        // Crear proceso hijo
        pids[i] = fork();
        
        if (pids[i] == -1) {
            perror("fork");
            // Limpiar procesos ya creados
            for (int j = 0; j < i; j++) {
                kill(pids[j], SIGTERM);
                waitpid(pids[j], NULL, 0);
            }
            free(pids);
            free_shared_memory(process_data, num_processes * sizeof(process_data_t));
            return -1.0;
        }
        else if (pids[i] == 0) {
            // Código del proceso hijo
            int result = process_matrix_multiply(A, B, C, &process_data[i]);
            exit(result);
        }
        // El proceso padre continúa el bucle para crear más hijos
    }
    
    // El proceso padre espera a todos los hijos
    int all_success = 1;
    for (int i = 0; i < num_processes; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        
        if (WIFEXITED(status)) {
            int exit_code = WEXITSTATUS(status);
            if (exit_code != 0) {
                printf("Proceso %d terminó con error: %d\n", i, exit_code);
                all_success = 0;
            }
        } else {
            printf("Proceso %d terminó anormalmente\n", i);
            all_success = 0;
        }
    }
    
    // Terminar medición de tiempo
    end_time = get_wall_time();
    
    // Liberar memoria
    free(pids);
    free_shared_memory(process_data, num_processes * sizeof(process_data_t));
    
    if (!all_success) {
        return -1.0;
    }
    
    printf("Tiempo paralelo (procesos) - Reloj: %.6f s\n", end_time - start_time);
    
    return end_time - start_time;
}

// Función para mostrar ayuda
void print_usage(char *program_name) {
    printf("Uso: %s <tamaño_matriz> [num_procesos] [semilla_A] [semilla_B]\n", program_name);
    printf("  tamaño_matriz: Tamaño de las matrices cuadradas (obligatorio)\n");
    printf("  num_procesos: Número de procesos a usar (opcional, por defecto: número de CPUs)\n");
    printf("  semilla_A: Semilla para generar matriz A (opcional, por defecto: tiempo actual)\n");
    printf("  semilla_B: Semilla para generar matriz B (opcional, por defecto: tiempo actual + 1)\n");
    printf("\nEjemplos:\n");
    printf("  %s 512           # Matriz 512x512, procesos automáticos\n", program_name);
    printf("  %s 1000 4        # Matriz 1000x1000, 4 procesos\n", program_name);
    printf("  %s 512 8 123 456 # Matriz 512x512, 8 procesos, semillas específicas\n", program_name);
}

// Función para verificar que los resultados son iguales
int verify_results(int *C_seq, int *C_par, int size) {
    for (int i = 0; i < size * size; i++) {
        if (C_seq[i] != C_par[i]) {
            int row = i / size;
            int col = i % size;
            printf("Error en verificación: C_seq[%d][%d]=%d != C_par[%d][%d]=%d\n",
                   row, col, C_seq[i], row, col, C_par[i]);
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int size, num_processes;
    int seed_A, seed_B;
    double parallel_time;
    
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
    
    // Obtener número de procesos
    if (argc >= 3) {
        num_processes = atoi(argv[2]);
        if (num_processes <= 0) {
            printf("Error: El número de procesos debe ser positivo.\n");
            return 1;
        }
    } else {
        // Usar número de CPUs por defecto
        num_processes = (int)sysconf(_SC_NPROCESSORS_ONLN);
        if (num_processes <= 0) num_processes = 4; // Fallback
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
    
    printf("=== Multiplicación de Matrices con Procesos ===\n");
    printf("Tamaño de matrices: %dx%d\n", size, size);
    printf("Número de procesos: %d\n", num_processes);
    printf("Semilla matriz A: %d\n", seed_A);
    printf("Semilla matriz B: %d\n", seed_B);
    printf("Allocando memoria compartida...\n");
    
    // Calcular tamaño total de memoria necesaria
    size_t matrix_size = size * size * sizeof(int);
    
    // Alocar memoria compartida para las matrices (usando representación contigua)
    int *A = (int*)allocate_shared_memory(matrix_size);
    int *B = (int*)allocate_shared_memory(matrix_size);
    int *C_sequential = (int*)allocate_shared_memory(matrix_size);
    int *C_parallel = (int*)allocate_shared_memory(matrix_size);
    
    if (A == NULL || B == NULL || C_sequential == NULL || C_parallel == NULL) {
        printf("Error: No se pudo alocar memoria compartida para las matrices.\n");
        if (A) free_shared_memory(A, matrix_size);
        if (B) free_shared_memory(B, matrix_size);
        if (C_sequential) free_shared_memory(C_sequential, matrix_size);
        if (C_parallel) free_shared_memory(C_parallel, matrix_size);
        return 1;
    }
    
    printf("Inicializando matrices con valores aleatorios...\n");
    
    // Inicializar matrices A y B con valores aleatorios
    initialize_matrix(A, size, seed_A);
    initialize_matrix(B, size, seed_B);
    
    // === EJECUCIÓN SECUENCIAL ===
    // printf("\n--- Ejecutando versión secuencial ---\n");
    // sequential_time = matrix_multiply_sequential(A, B, C_sequential, size);
    
    // if (sequential_time < 0) {
    //     printf("Error en ejecución secuencial\n");
    //     return 1;
    // }
    
    // === EJECUCIÓN PARALELA ===
    printf("\n--- Ejecutando versión paralela con procesos ---\n");
    parallel_time = matrix_multiply_parallel(A, B, C_parallel, size, num_processes);
    
    if (parallel_time < 0) {
        printf("Error en ejecución paralela\n");
        return 1;
    }
    
    // Calcular speedup
    // speedup = sequential_time / parallel_time;
    // double efficiency = (speedup / num_processes) * 100;
    
    printf("\n=== RESULTADOS ===\n");
    // printf("Tiempo secuencial: %.6f segundos\n", sequential_time);
    printf("Tiempo paralelo: %.6f segundos\n", parallel_time);
    // printf("Speedup: %.2fx\n", speedup);
    // printf("Eficiencia: %.2f%% (%d procesos)\n", efficiency, num_processes);
    // printf("GFLOPS secuencial: %.6f\n", (2.0 * size * size * size) / (sequential_time * 1e9));
    printf("GFLOPS paralelo: %.6f\n", (2.0 * size * size * size) / (parallel_time * 1e9));
    
    // Verificar que los resultados son correctos
    printf("\nVerificando resultados...\n");
    if (verify_results(C_sequential, C_parallel, size)) {
        printf("✓ Verificación exitosa: Ambos resultados son idénticos\n");
    } else {
        printf("✗ Error: Los resultados no coinciden\n");
    }
    
    // Calcular suma de verificación
    long long sum_seq = 0, sum_par = 0;
    for (int i = 0; i < size * size; i++) {
        sum_seq += C_sequential[i];
        sum_par += C_parallel[i];
    }
    printf("Suma verificación secuencial: %lld\n", sum_seq);
    printf("Suma verificación paralela: %lld\n", sum_par);
    
    // Liberar memoria compartida
    free_shared_memory(A, matrix_size);
    free_shared_memory(B, matrix_size);
    free_shared_memory(C_sequential, matrix_size);
    free_shared_memory(C_parallel, matrix_size);
    
    return 0;
}
