
#include <sys/time.h>    // Para medir el tiempo de ejecución
// Monte Carlo Dartboard - Paralelo con fork
#include <stdio.h>       // Entrada/salida estándar
#include <stdlib.h>      // Funciones estándar y generación de aleatorios
#include <math.h>        // Funciones matemáticas
#include <sys/wait.h>    // Para esperar procesos hijos
#include <unistd.h>      // Para fork, pipe
#include <time.h>        // Para inicializar la semilla

int DARTS = 10000000;   // Número total de lanzamientos
int PROCESSES = 4;      // Número de procesos a usar

// Simula los lanzamientos de dardos en un rango específico
int simulate(int start, int end, unsigned int seed) {
    int hits = 0; // Contador de aciertos
    for (int i = start; i < end; i++) { // Itera sobre el rango asignado
        double x = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0; // Genera coordenada x aleatoria entre -1 y 1
        double y = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0; // Genera coordenada y aleatoria entre -1 y 1
        if (x * x + y * y <= 1.0) hits++; // Si el punto cae dentro del círculo, suma
    }
    return hits; // Devuelve el número de aciertos en este rango
}

int main(int argc, char *argv[]) {
    struct timeval start, end;
    gettimeofday(&start, NULL); // Guarda el tiempo de inicio

    // Permite cambiar el número de lanzamientos y procesos por argumentos
    if (argc >= 2) DARTS = atoi(argv[1]);
    if (argc >= 3) PROCESSES = atoi(argv[2]);

    int chunk = DARTS / PROCESSES; // Cuántos lanzamientos hace cada proceso
    int total_hits = 0; // Contador global de aciertos
    int pipes[PROCESSES][2]; // Pipes para comunicación entre procesos

    // Crea procesos hijos y pipes para comunicación
    for (int i = 0; i < PROCESSES; i++) {
        if (pipe(pipes[i]) == -1) { // Crea un pipe para cada proceso
            perror("pipe error");
            exit(1);
        }
        pid_t pid = fork(); // Crea un proceso hijo
        if (pid == 0) { // Código que ejecuta el hijo
            unsigned int seed = time(NULL) ^ (i * 100); // Semilla única por proceso
            // Simula su parte de los lanzamientos
            int hits = simulate(i * chunk, (i == PROCESSES - 1) ? DARTS : (i + 1) * chunk, seed);
            // Escribe el resultado en el pipe
            if (write(pipes[i][1], &hits, sizeof(int)) == -1) {
                perror("write error");
                exit(1);
            }
            // Cierra los extremos del pipe y termina el hijo
            close(pipes[i][0]);
            close(pipes[i][1]);
            exit(0);
        }
        // El padre sigue creando más hijos
    }

    // El proceso padre recolecta los resultados de todos los hijos
    for (int i = 0; i < PROCESSES; i++) {
        int hits = 0;
        if (read(pipes[i][0], &hits, sizeof(int)) == -1) { // Lee el resultado del hijo
            perror("read error");
            exit(1);
        }
        close(pipes[i][0]); // Cierra el extremo de lectura
        close(pipes[i][1]); // Cierra el extremo de escritura
        total_hits += hits; // Suma los aciertos de este hijo al total
        wait(NULL); // Espera a que el hijo termine
    }

    gettimeofday(&end, NULL); // Guarda el tiempo de finalización

    // Calcula la estimación de pi igual que en la versión serial
    double pi = 4.0 * ((double)total_hits / DARTS);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Monte Carlo Dartboard (Fork): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
