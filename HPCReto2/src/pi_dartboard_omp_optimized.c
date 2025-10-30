// Monte Carlo Dartboard - OpenMP Optimized Version
// Incluye optimizaciones de CPU y memoria:
// - Precálculo de constantes
// - Minimizar operaciones
// - Evitar false sharing
// - Generadores aleatorios eficientes
// - Localidad de datos

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

// Estructura alineada para evitar false sharing
typedef struct {
    long value;
    char padding[64 - sizeof(long)];
} aligned_long_t __attribute__((aligned(64)));

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <iteraciones>\n", argv[0]);
        return 1;
    }
    
    long darts = atol(argv[1]);
    struct timeval start, end;
    
    // Precalcular constantes
    const double rand_max_inv = 1.0 / (double)RAND_MAX;
    
    gettimeofday(&start, NULL);

    // Obtener número de threads
    int num_threads;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }
    
    // Array alineado de contadores locales para evitar false sharing
    aligned_long_t *local_hits = (aligned_long_t*)calloc(num_threads, sizeof(aligned_long_t));
    
    // Paralelización optimizada
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        long my_hits = 0;
        
        // Semilla única por thread
        unsigned int seed = (unsigned int)(time(NULL) + tid * 12345);
        
        #pragma omp for schedule(static) nowait
        for (long i = 0; i < darts; i++) {
            // Generar coordenadas - optimizado para evitar multiplicaciones
            double x = ((double)rand_r(&seed) * rand_max_inv);
            double y = ((double)rand_r(&seed) * rand_max_inv);
            
            // Transformar a rango [-1, 1] usando suma en lugar de multiplicación
            x = x + x - 1.0;
            y = y + y - 1.0;
            
            // Verificar si está dentro del círculo
            // Uso directo de x*x + y*y evita sqrt innecesario
            if (x * x + y * y <= 1.0) {
                my_hits++;
            }
        }
        
        // Almacenar en estructura alineada
        local_hits[tid].value = my_hits;
    }
    
    // Reducción final
    long total_hits = 0;
    for (int i = 0; i < num_threads; i++) {
        total_hits += local_hits[i].value;
    }
    
    free(local_hits);
    
    gettimeofday(&end, NULL);

    double pi = 4.0 * ((double)total_hits / darts);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    printf("Monte Carlo Dartboard (OpenMP Optimized): PI estimado = %.8f\n", pi);
    printf("Threads utilizados: %d\n", num_threads);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    
    return 0;
}
