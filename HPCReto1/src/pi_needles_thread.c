#include <sys/time.h>
// Monte Carlo Needles (Buffon's Needle) - Paralelo con Threads
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>


#define LENGTH 1.0
#define DIST 1.0

int NEEDLES = 10000000;
int THREADS = 4;

typedef struct {
    int start, end, hits;
    unsigned int seed;
} ThreadData;

void* needle_sim(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int local_hits = 0;
    for (int i = data->start; i < data->end; i++) {
        double x = ((double)rand_r(&data->seed) / RAND_MAX) * (DIST / 2);
        double theta = ((double)rand_r(&data->seed) / RAND_MAX) * M_PI;
        double reach = (LENGTH / 2) * sin(theta);
        if (x <= reach) local_hits++;
    }
    data->hits = local_hits;
    return NULL;
}

int main(int argc, char *argv[]) {
    struct timeval start, end;
    gettimeofday(&start, NULL); // Inicio medición tiempo
    // Leer argumentos: iteraciones, hilos
    if (argc >= 2) NEEDLES = atoi(argv[1]);
    if (argc >= 3) THREADS = atoi(argv[2]);

    pthread_t threads[THREADS];
    ThreadData td[THREADS];
    int chunk = NEEDLES / THREADS;
    int total_hits = 0;
    // Crear hilos y asignarles su rango de simulación
    for (int i = 0; i < THREADS; i++) {
        td[i].start = i * chunk;
        td[i].end = (i == THREADS - 1) ? NEEDLES : (i + 1) * chunk;
        td[i].hits = 0;
        td[i].seed = time(NULL) ^ (i * 100); // Semilla única por hilo
        pthread_create(&threads[i], NULL, needle_sim, &td[i]);
    }
    // Esperar a que todos los hilos terminen y sumar resultados
    for (int i = 0; i < THREADS; i++) {
        pthread_join(threads[i], NULL);
        total_hits += td[i].hits;
    }
    gettimeofday(&end, NULL); // Fin medición tiempo
    double pi = (2.0 * LENGTH * NEEDLES) / (DIST * total_hits);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Buffon's Needle (Threads): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
