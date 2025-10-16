

// Monte Carlo Dartboard - Serial Version
#include <stdio.h>            // Incluye funciones de entrada/salida como printf
#include <stdlib.h>           // Incluye funciones para conversión y generación de números aleatorios
#include <math.h>             // Incluye funciones matemáticas como pow, sqrt
#include <time.h>             // Incluye funciones para trabajar con tiempo (semilla aleatoria)
#include <sys/time.h>         // Incluye funciones para medir el tiempo de ejecución

int main(int argc, char *argv[]) {
    // Verifica que el usuario haya pasado el número de iteraciones como argumento
    if (argc < 2) {
        printf("Uso: %s <iteraciones>\n", argv[0]); // Mensaje de uso si falta argumento
        return 1; // Termina el programa si no hay argumento
    }
    long darts = atol(argv[1]); // Convierte el argumento a número de lanzamientos (long)
    int hits = 0; // Inicializa el contador de aciertos dentro del círculo
    srand(time(NULL)); // Inicializa la semilla del generador de números aleatorios con el tiempo actual
    struct timeval start, end; // Estructuras para almacenar el tiempo de inicio y fin
    gettimeofday(&start, NULL); // Guarda el tiempo de inicio antes de la simulación

    // Bucle principal: simula cada lanzamiento de dardo
    for (long i = 0; i < darts; i++) {
        // Genera una coordenada x aleatoria entre -1 y 1
        double x = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        // Genera una coordenada y aleatoria entre -1 y 1
        double y = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
        // Calcula si el punto (x, y) está dentro del círculo de radio 1
        if (x * x + y * y <= 1.0) {
            hits++; // Si está dentro, incrementa el contador de aciertos
        }
    }

    gettimeofday(&end, NULL); // Guarda el tiempo de finalización después de la simulación

    // Calcula la estimación de pi usando la proporción de aciertos
    double pi = 4.0 * ((double)hits / darts);
    // Calcula el tiempo transcurrido en segundos
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;

    // Imprime el valor estimado de pi y el tiempo de ejecución
    printf("Monte Carlo Dartboard (Serial): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0; // Fin del programa
}
