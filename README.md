# Multiplicación de Matrices - High Performance Computing (HPC)

Este repositorio contiene implementaciones de multiplicación de matrices para el curso de High Performance Computing, incluyendo versiones secuenciales y paralelas con POSIX Threads.

## 🚀 Características

- **Versión Secuencial**: Implementación clásica O(n³)
- **Versión Paralela**: Concurrencia con POSIX Threads (pthreads)
- **Generación Aleatoria**: Matrices con valores enteros aleatorios y semillas configurables
- **Medición de Rendimiento**: Tiempo de usuario, tiempo de reloj y GFLOPS
- **Verificación**: Comparación entre resultados secuenciales y paralelos
- **Análisis de Speedup**: Cálculo de aceleración y eficiencia

## 📁 Archivos del Proyecto

### Código Fuente
- `matrix_multiplication.c` - Versión secuencial básica
- `matrix_multiplication_pthread.c` - Versión paralela con pthreads (completa)
- `matrix_multiplication_pthread_optimized.c` - Versión paralela optimizada
- `matrix_time_analysis.c` - Análisis completo de tiempos (usuario + reloj)

### Archivos de Configuración
- `Makefile` - Automatización de compilación y pruebas
- `test_matrix.sh` - Script de pruebas automatizado
- `README.md` - Documentación del proyecto
- `.gitignore` - Archivos ignorados por Git

## 🔧 Compilación

### Opción 1: Usando Makefile (Recomendado)

```bash
make                          # Versión secuencial
make matrix_mult_pthread      # Versión paralela
make matrix_mult_pthread_opt  # Versión paralela optimizada
make matrix_time_analysis     # Análisis completo de tiempos
make all                      # Compilar todas las versiones
```

### Opción 2: Compilación Manual

```bash
# Versión secuencial
gcc -O3 -Wall -Wextra -std=c99 -o matrix_mult matrix_multiplication.c

# Versión paralela
gcc -O3 -Wall -Wextra -std=c99 -pthread -o matrix_mult_pthread matrix_multiplication_pthread.c

# Versión optimizada
gcc -O3 -march=native -Wall -Wextra -std=c99 -pthread -o matrix_mult_pthread_opt matrix_multiplication_pthread_optimized.c
```

## 🎯 Uso

### Versión Secuencial
```bash
./matrix_mult <tamaño_matriz> [semilla_A] [semilla_B]

# Ejemplos
./matrix_mult 100           # Matriz 100x100 con semillas automáticas
./matrix_mult 512 123 456   # Matriz 512x512 con semillas específicas
```

### Versión Paralela
```bash
./matrix_mult_pthread <tamaño_matriz> [num_hilos] [semilla_A] [semilla_B]

# Ejemplos
./matrix_mult_pthread 1000            # Matriz 1000x1000, hilos automáticos
./matrix_mult_pthread 1000 8          # Matriz 1000x1000, 8 hilos
./matrix_mult_pthread 1000 8 123 456  # Con semillas específicas
```

### Análisis Completo
```bash
./matrix_time_analysis <tamaño_matriz> [num_hilos] [semilla_A] [semilla_B]

# Muestra tiempo de usuario, tiempo de reloj, speedup y eficiencia
```

## 🧪 Pruebas y Benchmarks

```bash
make test               # Pruebas básicas secuenciales
make test_pthread       # Pruebas con pthreads
make benchmark_compare  # Comparación secuencial vs paralelo
./test_matrix.sh        # Script de pruebas automatizado
```

## 📊 Medición de Rendimiento

El proyecto incluye mediciones de:

- **Tiempo de Usuario**: Tiempo real de CPU utilizado
- **Tiempo de Reloj**: Tiempo transcurrido (wall clock time)
- **GFLOPS**: Operaciones de punto flotante por segundo
- **Speedup**: Aceleración obtenida con paralelización
- **Eficiencia**: Porcentaje de utilización efectiva de hilos

## 💻 Compatibilidad

### macOS (Apple Silicon M1/M2)
- ✅ Compilador: Clang/GCC
- ✅ Optimizaciones específicas para ARM64
- ✅ POSIX Threads nativo
- ✅ Rendimiento excelente

### Linux
- ✅ GCC nativo
- ✅ POSIX Threads
- ✅ Optimizaciones específicas por arquitectura

## 🔍 Algoritmo y Estructura

### División de Trabajo
La paralelización se realiza por **división de filas**:
- Cada hilo procesa un subconjunto de filas de la matriz resultado
- Distribución automática balanceada
- Sin necesidad de sincronización durante el cómputo

### Estructura de Datos
```c
typedef struct {
    int **A, **B, **C;    // Matrices
    int size;             // Tamaño
    int start_row;        // Fila inicial
    int end_row;          // Fila final
    int thread_id;        // ID del hilo
} thread_data_t;
```

---
