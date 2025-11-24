// Ejecuta versión secuencial, pthreads y procesos (fork)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <sys/wait.h>

// ===================== Utilidades de tiempo =====================
static double get_user_time() {
    struct rusage usage; getrusage(RUSAGE_SELF, &usage);
    return usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1000000.0;
}
static double get_wall_time() {
    struct timeval tv; gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// ===================== Gestión de matrices (int ** estilo) =====================
static int **allocate_matrix(int n) {
    int **m = (int**)malloc(n * sizeof(int*));
    if(!m) return NULL;
    for(int i=0;i<n;i++){ m[i] = (int*)malloc(n * sizeof(int)); if(!m[i]) { for(int j=0;j<i;j++) free(m[j]); free(m); return NULL; } }
    return m;
}
static void free_matrix(int **m,int n){ if(!m) return; for(int i=0;i<n;i++) free(m[i]); free(m);} 
static void initialize_matrix(int **m,int n,int seed){ srand(seed); for(int i=0;i<n;i++) for(int j=0;j<n;j++) m[i][j]=rand()%100; }

// ===================== Secuencial =====================
static void matmul_seq(int **A,int **B,int **C,int n){
    for(int i=0;i<n;i++){
        for(int j=0;j<n;j++){
            int sum=0; for(int k=0;k<n;k++) sum += A[i][k]*B[k][j];
            C[i][j]=sum;
        }
    }
}

// ===================== Pthreads =====================
typedef struct { int **A, **B, **C; int n; int start_row; int end_row; } thread_data_t;
static void* thread_worker(void *arg){
    thread_data_t *d=(thread_data_t*)arg;
    for(int i=d->start_row;i<d->end_row;i++){
        for(int j=0;j<d->n;j++){
            int sum=0; for(int k=0;k<d->n;k++) sum += d->A[i][k]*d->B[k][j];
            d->C[i][j]=sum;
        }
    }
    return NULL;
}
static void matmul_pthreads(int **A,int **B,int **C,int n,int num_threads){
    pthread_t *threads = (pthread_t*)malloc(num_threads*sizeof(pthread_t));
    thread_data_t *td = (thread_data_t*)malloc(num_threads*sizeof(thread_data_t));
    int base = n / num_threads; int rem = n % num_threads; int row=0;
    for(int t=0;t<num_threads;t++){
        int extra = (t<rem)?1:0; int start=row; int end = start + base + extra; row=end;
        td[t].A=A; td[t].B=B; td[t].C=C; td[t].n=n; td[t].start_row=start; td[t].end_row=end;
        pthread_create(&threads[t], NULL, thread_worker, &td[t]);
    }
    for(int t=0;t<num_threads;t++) pthread_join(threads[t], NULL);
    free(threads); free(td);
}

// ===================== Procesos (fork + mmap) =====================
static void child_proc(int *A,int *B,int *C,int n,int start,int end){
    for(int i=start;i<end;i++){
        int row_off = i*n;
        for(int j=0;j<n;j++){
            int sum=0; for(int k=0;k<n;k++) sum += A[row_off + k]*B[k*n + j];
            C[row_off + j]=sum;
        }
    }
    _exit(0);
}
static void matmul_process(int *A,int *B,int *C,int n,int num_procs){
    int base = n / num_procs; int rem = n % num_procs; int row=0;
    for(int p=0;p<num_procs;p++){
        int extra=(p<rem)?1:0; int start=row; int end=start+base+extra; row=end;
        pid_t pid=fork();
        if(pid<0){ perror("fork"); exit(1);} else if(pid==0){ child_proc(A,B,C,n,start,end); }
    }
    while(wait(NULL)>0){} // esperar todos
}

// ===================== Verificación =====================
static int verify_all(int **C_seq,int **C_thr,int *C_proc,int n){
    for(int i=0;i<n;i++) for(int j=0;j<n;j++){
        int vseq=C_seq[i][j]; int vthr=C_thr[i][j]; int vproc=C_proc[i*n + j];
        if(vseq!=vthr || vseq!=vproc){
            fprintf(stderr,"Diferencia en (%d,%d): seq=%d thr=%d proc=%d\n", i,j,vseq,vthr,vproc);
            return 0;
        }
    }
    return 1;
}

// ===================== Programa Principal =====================
static void usage(const char *p){
    printf("Uso: %s <tamaño_matriz> [num_trabajadores] [semilla_A] [semilla_B]\n", p);
    printf("Ejemplo: %s 1024 8 123 456\n", p);
}

int main(int argc,char *argv[]){
    if(argc<2 || argc>5){ usage(argv[0]); return 1; }
    int n = atoi(argv[1]); if(n<=0){ fprintf(stderr,"Tamaño inválido\n"); return 1; }
    int workers;
    if(argc>=3){ workers=atoi(argv[2]); if(workers<=0) { fprintf(stderr,"Trabajadores inválidos\n"); return 1; } }
    else { workers = (int)sysconf(_SC_NPROCESSORS_ONLN); if(workers<=0) workers=2; }
    int seedA = (argc>=4)?atoi(argv[3]):(int)time(NULL);
    int seedB = (argc==5)?atoi(argv[4]):seedA+1;

    printf("=== Multiplicación de Matrices: Secuencial vs Hilos vs Procesos ===\n");
    printf("Tamaño: %d x %d\n", n,n);
    printf("Trabajadores (hilos/procesos): %d\n", workers);
    printf("Semillas: A=%d B=%d\n", seedA, seedB);

    // Matrices para seq/pthreads
    int **A = allocate_matrix(n); int **B = allocate_matrix(n); int **C_seq = allocate_matrix(n); int **C_thr = allocate_matrix(n);
    if(!A||!B||!C_seq||!C_thr){ fprintf(stderr,"Fallo al reservar memoria (int**)\n"); return 1; }
    initialize_matrix(A,n,seedA); initialize_matrix(B,n,seedB);

    // Memoria compartida para procesos (contigua)
    size_t bytes = (size_t)n * n * sizeof(int);
    int *A1 = mmap(NULL, bytes, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int *B1 = mmap(NULL, bytes, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    int *C_proc = mmap(NULL, bytes, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    if(A1==MAP_FAILED||B1==MAP_FAILED||C_proc==MAP_FAILED){ perror("mmap"); return 1; }
    // Copiar A,B al formato 1D
    for(int i=0;i<n;i++) for(int j=0;j<n;j++){ A1[i*n+j]=A[i][j]; B1[i*n+j]=B[i][j]; }

    // ===== Secuencial =====
    printf("\n--- Secuencial ---\n");
    double s_user = get_user_time(); double s_wall = get_wall_time();
    matmul_seq(A,B,C_seq,n);
    double e_user = get_user_time(); double e_wall = get_wall_time();
    double seq_user = e_user - s_user; double seq_wall = e_wall - s_wall;
    printf("Tiempo usuario: %.6f s\n", seq_user);
    printf("Tiempo pared : %.6f s\n", seq_wall);
    printf("GFLOPS (wall): %.6f\n", (2.0 * n * (double)n * (double)n) / (seq_wall * 1e9));

    // ===== Pthreads =====
    printf("\n--- Paralelo (Hilos) ---\n");
    s_user = get_user_time(); s_wall = get_wall_time();
    matmul_pthreads(A,B,C_thr,n,workers);
    e_user = get_user_time(); e_wall = get_wall_time();
    double thr_user = e_user - s_user; double thr_wall = e_wall - s_wall;
    printf("Tiempo usuario: %.6f s\n", thr_user);
    printf("Tiempo pared : %.6f s\n", thr_wall);
    printf("GFLOPS (wall): %.6f\n", (2.0 * n * (double)n * (double)n) / (thr_wall * 1e9));
    double speedup_thr = seq_wall / thr_wall;
    printf("Speedup (wall): %.2fx\n", speedup_thr);
    printf("Eficiencia: %.2f%%\n", (speedup_thr / workers) * 100.0);

    // ===== Procesos =====
    printf("\n--- Paralelo (Procesos) ---\n");
    s_user = get_user_time(); s_wall = get_wall_time();
    matmul_process(A1,B1,C_proc,n,workers);
    e_user = get_user_time(); e_wall = get_wall_time();
    double proc_user = e_user - s_user; double proc_wall = e_wall - s_wall;
    printf("Tiempo usuario (padre): %.6f s\n", proc_user);
    printf("Tiempo pared           : %.6f s\n", proc_wall);
    printf("GFLOPS (wall): %.6f\n", (2.0 * n * (double)n * (double)n) / (proc_wall * 1e9));
    double speedup_proc = seq_wall / proc_wall;
    printf("Speedup (wall): %.2fx\n", speedup_proc);
    printf("Eficiencia: %.2f%%\n", (speedup_proc / workers) * 100.0);

    // ===== Verificación =====
    printf("\nVerificando resultados...\n");
    if(verify_all(C_seq,C_thr,C_proc,n)) printf("✓ Resultados idénticos en las tres versiones\n"); else printf("✗ Diferencias detectadas\n");

    long long sum_seq=0,sum_thr=0,sum_proc=0;
    for(int i=0;i<n;i++) for(int j=0;j<n;j++){ sum_seq += C_seq[i][j]; sum_thr += C_thr[i][j]; sum_proc += C_proc[i*n + j]; }
    printf("Suma secuencial: %lld\n", sum_seq);
    printf("Suma hilos     : %lld\n", sum_thr);
    printf("Suma procesos  : %lld\n", sum_proc);

    printf("\n=== RESUMEN (Wall) ===\n");
    printf("Secuencial: %.6f s\n", seq_wall);
    printf("Hilos     : %.6f s  (Speedup %.2fx)\n", thr_wall, speedup_thr);
    printf("Procesos  : %.6f s  (Speedup %.2fx)\n", proc_wall, speedup_proc);
    if(thr_wall < proc_wall) printf("Mejor: Hilos\n"); else if(proc_wall < thr_wall) printf("Mejor: Procesos\n"); else printf("Empate\n");

    // Liberar memoria
    free_matrix(A,n); free_matrix(B,n); free_matrix(C_seq,n); free_matrix(C_thr,n);
    munmap(A1,bytes); munmap(B1,bytes); munmap(C_proc,bytes);
    return 0;
}
