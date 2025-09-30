#include <sys/time.h>
// Monte Carlo Needles (Buffon's Needle) - Serial Version
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define NEEDLES 10000000
#define LENGTH 1.0
#define DIST 1.0

int main() {
    int hits = 0;
    srand(time(NULL));
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < NEEDLES; i++) {
        double x = ((double)rand() / RAND_MAX) * (DIST / 2);
        double theta = ((double)rand() / RAND_MAX) * M_PI;
        double reach = (LENGTH / 2) * sin(theta);
        if (x <= reach) hits++;
    }
    gettimeofday(&end, NULL);
    double pi = (2.0 * LENGTH * NEEDLES) / (DIST * hits);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Buffon's Needle (Serial): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
