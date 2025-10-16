// calcula_speedup.c
// Lee benchmarksReto1.csv, calcula speedup y genera speedups.csv
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 512
#define MAX_ROWS 10000

typedef struct {
    char algoritmo[32];
    char tipo[32];
    int hilos;
    long iteraciones;
    double tiempo;
} Benchmark;

int main() {
    FILE *fin = fopen("benchmarksReto1.csv", "r");
    FILE *fout = fopen("speedups.csv", "w");
    if (!fin || !fout) {
        printf("Error abriendo archivos.\n");
        return 1;
    }
    char line[MAX_LINE];
    Benchmark rows[MAX_ROWS];
    int n = 0;
    fgets(line, MAX_LINE, fin); // Saltar encabezado
    while (fgets(line, MAX_LINE, fin)) {
        char *tok;
        tok = strtok(line, ",");
        strcpy(rows[n].algoritmo, tok);
        tok = strtok(NULL, ",");
        strcpy(rows[n].tipo, tok);
        tok = strtok(NULL, ",");
        rows[n].hilos = atoi(tok);
        tok = strtok(NULL, ",");
        rows[n].iteraciones = atol(tok);
        tok = strtok(NULL, ",");
        rows[n].tiempo = atof(tok);
        n++;
    }
    fprintf(fout, "Algoritmo,Tipo,Hilos/Procesos,Iteraciones,Tiempo(s),Speedup\n");
    for (int i = 0; i < n; i++) {
        double serial_time = -1.0;
        // Buscar tiempo serial correspondiente
        for (int j = 0; j < n; j++) {
            if (strcmp(rows[i].algoritmo, rows[j].algoritmo) == 0 &&
                strcmp(rows[j].tipo, "Serial") == 0 &&
                rows[i].iteraciones == rows[j].iteraciones) {
                serial_time = rows[j].tiempo;
                break;
            }
        }
        double speedup = (serial_time > 0 && rows[i].tiempo > 0) ? serial_time / rows[i].tiempo : 0.0;
        fprintf(fout, "%s,%s,%d,%ld,%.6f,%.4f\n",
            rows[i].algoritmo, rows[i].tipo, rows[i].hilos, rows[i].iteraciones, rows[i].tiempo, speedup);
    }
    fclose(fin);
    fclose(fout);
    printf("Archivo speedups.csv generado correctamente.\n");
    return 0;
}
