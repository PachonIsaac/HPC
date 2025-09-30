// Monte Carlo Dartboard - Serial Version
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <iteraciones>\n", argv[0]);
        return 1;
    }
    long darts = atol(argv[1]);
    int hits = 0;
    srand(time(NULL));
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (long i = 0; i < darts; i++) {
        double x = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        double y = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        if (x * x + y * y <= 1.0) hits++;
    }
    gettimeofday(&end, NULL);
    double pi = 4.0 * ((double)hits / darts);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Monte Carlo Dartboard (Serial): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
