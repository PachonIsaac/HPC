// Monte Carlo Buffon's Needle - Cache-Optimized with sys/time.h
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <limits.h>
#include <time.h>

#define NEEDLES 10000000
#define BLOCK 10000
#define LENGTH 1.0
#define DIST 1.0

static inline unsigned int fast_rand(unsigned int *state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

int main(int argc, char *argv[]) {
    int needles = NEEDLES;
    if (argc >= 2) needles = atoi(argv[1]);
    int hits = 0;
    unsigned int seed = (unsigned int)time(NULL);
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for (int i = 0; i < needles; i += BLOCK) {
        int local_hits = 0;
        for (int j = 0; j < BLOCK && (i + j) < needles; j++) {
            double x = ((double)fast_rand(&seed) / UINT_MAX) * (DIST / 2);
            double theta = ((double)fast_rand(&seed) / UINT_MAX) * M_PI;
            double reach = (LENGTH * 0.5) * sin(theta);
            local_hits += (x <= reach);
        }
        hits += local_hits;
    }
    gettimeofday(&end, NULL);
    double pi = (2.0 * LENGTH * needles) / (DIST * hits);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Buffon's Needle (Cache-Optimized): PI = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
