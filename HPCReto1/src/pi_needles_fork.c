#include <sys/time.h>
// Monte Carlo Needles (Buffon's Needle) - Paralelo con fork
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>


#define LENGTH 1.0
#define DIST 1.0

int NEEDLES = 10000000;
int PROCESSES = 4;

int simulate(int start, int end, unsigned int seed) {
    int hits = 0;
    for (int i = start; i < end; i++) {
        double x = ((double)rand_r(&seed) / RAND_MAX) * (DIST / 2);
        double theta = ((double)rand_r(&seed) / RAND_MAX) * M_PI;
        double reach = (LENGTH / 2) * sin(theta);
        if (x <= reach) hits++;
    }
    return hits;
}

int main(int argc, char *argv[]) {
    struct timeval start, end;
    gettimeofday(&start, NULL); // Inicio medición tiempo
    // Leer argumentos: iteraciones, procesos
    if (argc >= 2) NEEDLES = atoi(argv[1]);
    if (argc >= 3) PROCESSES = atoi(argv[2]);

    int chunk = NEEDLES / PROCESSES;
    int total_hits = 0;
    int pipes[PROCESSES][2];
    // Crear procesos hijos y pipes para comunicación
    for (int i = 0; i < PROCESSES; i++) {

        #include <sys/time.h>    // Para medir el tiempo de ejecución
        // Monte Carlo Needles (Buffon's Needle) - Paralelo con fork
        #include <stdio.h>       // Entrada/salida estándar
        #include <stdlib.h>      // Funciones estándar y generación de aleatorios
        #include <math.h>        // Funciones matemáticas
        #include <sys/wait.h>    // Para esperar procesos hijos
        #include <unistd.h>      // Para fork, pipe
        #include <time.h>        // Para inicializar la semilla

        #define LENGTH 1.0       // Longitud de la aguja
        #define DIST 1.0         // Distancia entre líneas

        int NEEDLES = 10000000;  // Número total de lanzamientos
        int PROCESSES = 4;       // Número de procesos a usar

        // Simula los lanzamientos de agujas en un rango específico
        int simulate(int start, int end, unsigned int seed) {
            int hits = 0; // Contador de cruces
            for (int i = start; i < end; i++) { // Itera sobre el rango asignado
                double x = ((double)rand_r(&seed) / RAND_MAX) * (DIST / 2); // Posición centro
                double theta = ((double)rand_r(&seed) / RAND_MAX) * M_PI;   // Ángulo aleatorio
                double reach = (LENGTH / 2) * sin(theta);                   // Proyección
                if (x <= reach) hits++; // Si cruza línea, suma
            }
            return hits; // Devuelve el número de cruces en este rango
        }

        int main(int argc, char *argv[]) {
            struct timeval start, end;
            gettimeofday(&start, NULL); // Guarda el tiempo de inicio

            // Permite cambiar el número de lanzamientos y procesos por argumentos
            if (argc >= 2) NEEDLES = atoi(argv[1]);
            if (argc >= 3) PROCESSES = atoi(argv[2]);

            int chunk = NEEDLES / PROCESSES; // Cuántos lanzamientos hace cada proceso
            int total_hits = 0; // Contador global de cruces
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
                    int hits = simulate(i * chunk, (i == PROCESSES - 1) ? NEEDLES : (i + 1) * chunk, seed);
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
                total_hits += hits; // Suma los cruces de este hijo al total
            }

            // Espera a que todos los procesos hijos terminen
            for (int i = 0; i < PROCESSES; i++) wait(NULL);

            gettimeofday(&end, NULL); // Guarda el tiempo de finalización

            // Calcula la estimación de pi igual que en la versión serial
            double pi = (2.0 * LENGTH * NEEDLES) / (DIST * total_hits);
            double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
            printf("Buffon's Needle (Fork): PI estimado = %.8f\n", pi);
            printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
            return 0;
        }
