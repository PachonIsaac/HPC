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
    gettimeofday(&start, NULL);
    // Leer argumentos: iteraciones, procesos
    if (argc >= 2) NEEDLES = atoi(argv[1]);
    if (argc >= 3) PROCESSES = atoi(argv[2]);

    int chunk = NEEDLES / PROCESSES;
    int total_hits = 0;
    int pipes[PROCESSES][2];
    for (int i = 0; i < PROCESSES; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe error");
            exit(1);
        }
        pid_t pid = fork();
        if (pid == 0) {
            unsigned int seed = time(NULL) ^ (i * 100);
            int hits = simulate(i * chunk, (i == PROCESSES - 1) ? NEEDLES : (i + 1) * chunk, seed);
            if (write(pipes[i][1], &hits, sizeof(int)) == -1) {
                perror("write error");
                exit(1);
            }
            close(pipes[i][0]);
            close(pipes[i][1]);
            exit(0);
        }
    }
    for (int i = 0; i < PROCESSES; i++) {
        int hits = 0;
        if (read(pipes[i][0], &hits, sizeof(int)) == -1) {
            perror("read error");
            exit(1);
        }
        close(pipes[i][0]);
        close(pipes[i][1]);
        total_hits += hits;
        wait(NULL);
    }
    gettimeofday(&end, NULL);
    double pi = (2.0 * LENGTH * NEEDLES) / (DIST * total_hits);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
    printf("Buffon's Needle (Fork): PI estimado = %.8f\n", pi);
    printf("Tiempo de ejecución: %.6f segundos\n", elapsed);
    return 0;
}
