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

// Función que simula el lanzamiento de dardos en un rango específico
void* dartboard_sim(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int local_hits = 0;
    for (int i = data->start; i < data->end; i++) { // Itera sobre el rango asignado al hilo
        double x = ((double)rand_r(&data->seed) / RAND_MAX) * 2.0 - 1.0; // Genera coordenada x aleatoria entre -1 y 1
        double y = ((double)rand_r(&data->seed) / RAND_MAX) * 2.0 - 1.0; // Genera coordenada y aleatoria entre -1 y 1
        if (x * x + y * y <= 1.0) // Verifica si el punto cae dentro del círculo
            local_hits++; // Si está dentro, incrementa el contador local
    }
    data->hits = local_hits;
    return NULL;
}

int main(int argc, char *argv[]) {
    struct timeval start, end;
    gettimeofday(&start, NULL); // Inicio medición tiempo
    // Leer argumentos: iteraciones, hilos
    if (argc >= 2) DARTS = atoi(argv[1]);
    if (argc >= 3) THREADS = atoi(argv[2]);

    pthread_t threads[THREADS];
    ThreadData td[THREADS];
    int chunk = DARTS / THREADS;
    int total_hits = 0;
    // Crear hilos y asignarles su rango de simulación
    for (int i = 0; i < THREADS; i++) { // Crea cada hilo
        td[i].start = i * chunk; // Asigna el inicio del rango de simulación
        td[i].end = (i == THREADS - 1) ? DARTS : (i + 1) * chunk; // Asigna el final del rango
        td[i].hits = 0; // Inicializa el contador de aciertos
        td[i].seed = time(NULL) ^ (i * 100); // Semilla única por hilo
        pthread_create(&threads[i], NULL, dartboard_sim, &td[i]); // Crea el hilo y le pasa los datos
    }
    // Esperar a que todos los hilos terminen y sumar resultados
    for (int i = 0; i < THREADS; i++) { // Espera a que cada hilo termine
        pthread_join(threads[i], NULL); // Espera la finalización del hilo
        total_hits += td[i].hits; // Suma los aciertos de cada hilo al total
    }
    gettimeofday(&end, NULL); // Fin medición tiempo
    double pi = 4.0 * ((double)total_hits / DARTS);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Monte Carlo Dartboard (Threads): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
