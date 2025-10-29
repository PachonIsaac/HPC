
// Monte Carlo Dartboard - Versión Serial Optimizada (mejor uso de caché y generación rápida de aleatorios)
#include <stdio.h>      // Entrada/salida estándar
#include <stdlib.h>     // Funciones estándar y generación de aleatorios
#include <math.h>       // Funciones matemáticas
#include <sys/time.h>   // Medición de tiempo
#include <limits.h>     // Para UINT_MAX
#include <time.h>       // Para inicializar la semilla

#define DARTS 10000000 // Número total de lanzamientos
#define BLOCK 10000    // Tamaño de bloque para procesamiento por lotes

// Generador rápido de números aleatorios (más eficiente que rand())
static inline unsigned int fast_rand(unsigned int *state) {
    unsigned int x = *state;
    x ^= x << 13;   // Mezcla de bits
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

int main(int argc, char *argv[]) {
    int darts = DARTS;
    if (argc >= 2) darts = atoi(argv[1]); // Permite cambiar el número de lanzamientos por argumento
    int hits = 0; // Contador de aciertos
    unsigned int seed = (unsigned int)time(NULL); // Semilla para fast_rand
    struct timeval start, end;
    gettimeofday(&start, NULL); // Inicio medición tiempo

    // Procesamiento por bloques: mejora el uso de caché y reduce la sobrecarga de bucles
    for (int i = 0; i < darts; i += BLOCK) {
        int local_hits = 0; // Contador local para el bloque
        for (int j = 0; j < BLOCK && (i + j) < darts; j++) {
            // Genera coordenadas aleatorias usando fast_rand
            double x = ((double)fast_rand(&seed) / UINT_MAX) * 2.0 - 1.0;
            double y = ((double)fast_rand(&seed) / UINT_MAX) * 2.0 - 1.0;
            // Suma si el punto cae dentro del círculo
            local_hits += (x * x + y * y <= 1.0);
        }
        hits += local_hits; // Acumula los aciertos del bloque
    }
    gettimeofday(&end, NULL); // Fin medición tiempo

    // Estimación de pi igual que la versión serial
    double pi = 4.0 * ((double)hits / darts);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Monte Carlo Dartboard (Cache-Optimized): PI = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
