// Monte Carlo Dartboard - OpenMP Static Scheduling
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <iteraciones>\n", argv[0]);
        return 1;
    }
    
    long darts = atol(argv[1]);
    long hits = 0;
    struct timeval start, end;
    
    gettimeofday(&start, NULL);

    // Static scheduling: distribución estática de iteraciones
    #pragma omp parallel for reduction(+:hits) schedule(static)
    for (long i = 0; i < darts; i++) {
        unsigned int seed = (unsigned int)(i + omp_get_thread_num() * darts);
        
        double x = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        double y = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        
        if (x * x + y * y <= 1.0) {
            hits++;
        }
    }

    gettimeofday(&end, NULL);

    double pi = 4.0 * ((double)hits / darts);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    
    int num_threads = 1;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }

    printf("Monte Carlo Dartboard (OpenMP Static): PI estimado = %.8f\n", pi);
    printf("Threads utilizados: %d\n", num_threads);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    
    return 0;
}
