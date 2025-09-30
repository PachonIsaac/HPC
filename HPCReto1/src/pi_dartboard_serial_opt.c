// Monte Carlo Dartboard - Cache-Optimized with sys/time.h
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#include <limits.h>
#include <time.h>

#define DARTS 10000000
#define BLOCK 10000

static inline unsigned int fast_rand(unsigned int *state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

int main(int argc, char *argv[]) {
    int darts = DARTS;
    if (argc >= 2) darts = atoi(argv[1]);
    int hits = 0;
    unsigned int seed = (unsigned int)time(NULL);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < darts; i += BLOCK) {
        int local_hits = 0;
        for (int j = 0; j < BLOCK && (i + j) < darts; j++) {
            double x = ((double)fast_rand(&seed) / UINT_MAX) * 2.0 - 1.0;
            double y = ((double)fast_rand(&seed) / UINT_MAX) * 2.0 - 1.0;
            local_hits += (x * x + y * y <= 1.0);
        }
        hits += local_hits;
    }
    gettimeofday(&end, NULL);
    double pi = 4.0 * ((double)hits / darts);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Monte Carlo Dartboard (Cache-Optimized): PI = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
