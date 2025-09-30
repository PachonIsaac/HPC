#include <sys/time.h>
// Monte Carlo Dartboard - Paralelo con Threads
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>


int DARTS = 10000000;
int THREADS = 4;

typedef struct {
    int start, end, hits;
    unsigned int seed;
} ThreadData;

void* dartboard_sim(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int local_hits = 0;
    for (int i = data->start; i < data->end; i++) {
        double x = ((double)rand_r(&data->seed) / RAND_MAX) * 2.0 - 1.0;
        double y = ((double)rand_r(&data->seed) / RAND_MAX) * 2.0 - 1.0;
        if (x * x + y * y <= 1.0) local_hits++;
    }
    data->hits = local_hits;
    return NULL;
}

int main(int argc, char *argv[]) {
    struct timeval start, end;
    gettimeofday(&start, NULL);
    // Leer argumentos: iteraciones, hilos
    if (argc >= 2) DARTS = atoi(argv[1]);
    if (argc >= 3) THREADS = atoi(argv[2]);

    pthread_t threads[THREADS];
    ThreadData td[THREADS];
    int chunk = DARTS / THREADS;
    int total_hits = 0;
    for (int i = 0; i < THREADS; i++) {
        td[i].start = i * chunk;
        td[i].end = (i == THREADS - 1) ? DARTS : (i + 1) * chunk;
        td[i].hits = 0;
        td[i].seed = time(NULL) ^ (i * 100);
        pthread_create(&threads[i], NULL, dartboard_sim, &td[i]);
    }
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
        total_hits += td[i].hits;
    }
    gettimeofday(&end, NULL);
    double pi = 4.0 * ((double)total_hits / DARTS);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Monte Carlo Dartboard (Threads): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
