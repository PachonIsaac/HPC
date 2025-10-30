// Monte Carlo Needles (Buffon's Needle) - OpenMP Guided Scheduling
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>

#define LENGTH 1.0
#define DIST 1.0

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <iteraciones>\n", argv[0]);
        return 1;
    }
    
    long needles = atol(argv[1]);
    long hits = 0;
    struct timeval start, end;
    
    gettimeofday(&start, NULL);

    // Guided scheduling: chunks adaptativos (grandes al inicio, pequeños al final)
    // Compromiso entre static y dynamic
    #pragma omp parallel for reduction(+:hits) schedule(guided)
    for (long i = 0; i < needles; i++) {
        unsigned int seed = (unsigned int)(i + omp_get_thread_num() * needles);
        
        double x = ((double)rand_r(&seed) / RAND_MAX) * (DIST / 2);
        double theta = ((double)rand_r(&seed) / RAND_MAX) * M_PI;
        double reach = (LENGTH / 2) * sin(theta);
        
        if (x <= reach) {
            hits++;
        }
    }

    gettimeofday(&end, NULL);

    double pi = (2.0 * LENGTH * needles) / (DIST * hits);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    
    int num_threads = 1;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }

    printf("Buffon's Needle (OpenMP Guided): PI estimado = %.8f\n", pi);
    printf("Threads utilizados: %d\n", num_threads);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    
    return 0;
}
