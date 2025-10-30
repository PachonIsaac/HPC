
#include <sys/time.h>         // Para medir el tiempo de ejecución
// Monte Carlo Needles (Buffon's Needle) - Serial Version
#include <stdio.h>            // Para funciones de entrada/salida
#include <stdlib.h>           // Para funciones de conversión y generación de números aleatorios
#include <math.h>             // Para funciones matemáticas (como sin y M_PI)
#include <time.h>             // Para inicializar la semilla aleatoria

#define LENGTH 1.0            // Longitud de la aguja
#define DIST 1.0              // Distancia entre líneas

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <iteraciones>\n", argv[0]);
        return 1;
    }
    long needles = atol(argv[1]); // Número de lanzamientos
    int hits = 0;
    srand(time(NULL)); // Semilla aleatoria
    struct timeval start, end;
    gettimeofday(&start, NULL); // Inicio medición tiempo

    // Simulación principal: lanzar agujas y contar cruces
    for (long i = 0; i < needles; i++) {
        double x = ((double)rand() / RAND_MAX) * (DIST / 2); // Posición centro
        double theta = ((double)rand() / RAND_MAX) * M_PI;   // Ángulo aleatorio
        double reach = (LENGTH / 2) * sin(theta);            // Proyección
        if (x <= reach) hits++; // Cruza línea
    }

    gettimeofday(&end, NULL); // Fin medición tiempo

    double pi = (2.0 * LENGTH * needles) / (DIST * hits); // Estimación de pi
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6; // Tiempo

    printf("Buffon's Needle (Serial): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
