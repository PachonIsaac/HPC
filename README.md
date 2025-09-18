# Multiplicación de Matrices - High Performance Computing (HPC)

Este repositorio contiene implementaciones de multiplicación de matrices para el curso de High Performance Computing, incluyendo versiones secuenciales, paralelas con POSIX Threads y ahora una versión paralela con procesos (`fork`) usando memoria compartida.

## 🚀 Características

- **Versión Secuencial**: Implementación clásica O(n³)
- **Versión Paralela Hilos (pthreads)**: División de filas, balanceo simple
- **Versión Paralela Procesos (fork + mmap)**: (Integrada ahora dentro del ejecutable unificado)
- **Versión Pthreads Optimizada**: Variantes con optimizaciones extra
- **Generación Aleatoria**: Matrices con valores enteros aleatorios y semillas configurables
- **Medición de Rendimiento**: Tiempo de usuario, tiempo de pared y GFLOPS
- **Verificación**: Comparación entre resultados secuenciales y paralelos
- **Análisis de Speedup**: Cálculo de aceleración y eficiencia (wall time)

## 📁 Archivos del Proyecto

### Código Fuente
- `matrix_multiplication.c` - Versión secuencial básica
- `matrix_multiplication_pthread.c` - Versión paralela con pthreads (completa)
- `matrix_multiplication_pthread_optimized.c` - Versión paralela optimizada
- (Integrada) Versión con procesos ahora incluida dentro de `matrix_mult_all` (antes `matrix_multiplication_process.c`)
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
make matrix_mult_pthread      # Versión paralela (hilos)
make matrix_mult_pthread_opt  # Versión paralela optimizada
make matrix_mult_all          # Ejecuta secuencial + hilos + procesos
make matrix_time_analysis     # Análisis completo de tiempos
make all                      # Compilar todas las versiones
```

La versión con procesos dedicada se ha eliminado. Usa `matrix_mult_all` para correr las tres variantes.

### Opción 2: Compilación Manual

```bash
# Versión secuencial
gcc -O3 -Wall -Wextra -std=c99 -o matrix_mult matrix_multiplication.c

# Versión paralela con hilos
gcc -O3 -Wall -Wextra -std=c99 -pthread -o matrix_mult_pthread matrix_multiplication_pthread.c

# Versión paralela optimizada (hilos)
gcc -O3 -march=native -Wall -Wextra -std=c99 -pthread -o matrix_mult_pthread_opt matrix_multiplication_pthread_optimized.c

# (La variante de procesos está incluida en matrix_mult_all)
```

## 🎯 Uso

### Versión Secuencial
```bash
./matrix_mult <tamaño> [semilla_A] [semilla_B]
# Ejemplos
./matrix_mult 100
./matrix_mult 512 123 456
```

### Versión Paralela con Hilos
```bash
./matrix_mult_pthread <tamaño> [num_hilos] [semilla_A] [semilla_B]
# Ejemplos
./matrix_mult_pthread 1000
./matrix_mult_pthread 1000 8
./matrix_mult_pthread 1000 8 123 456
```

### Versión Comparativa Unificada (Secuencial + Hilos + Procesos)
```bash
./matrix_mult_all <tamaño> [trabajadores] [semilla_A] [semilla_B]
# Ejemplos
./matrix_mult_all 1024
./matrix_mult_all 1024 8
./matrix_mult_all 1024 8 123 456
```

Notas:
- Si no se indica `num_hilos` o `num_procesos`, se usa el número de CPUs detectadas.
- Las semillas garantizan reproducibilidad entre ejecuciones y entre versiones (útil para comparar).

## 🧪 Pruebas y Benchmarks

```bash
make test                  # Pruebas básicas secuenciales
make test_pthread          # Pruebas con pthreads
make benchmark_compare     # Comparación secuencial vs paralelo (si está definido)
./test_matrix.sh           # Script de pruebas automatizado
```

Para comparar hilos vs procesos ahora basta una sola ejecución:
```bash
./matrix_mult_all 1600 8 123 456
```

## 📊 Métricas y Análisis

Se reportan:
- Tiempo de pared (wall time) – base para speedup.
- Tiempo de usuario (en procesos solo refleja el padre; usar `time -v` para sumar hijos si se desea).</n- GFLOPS aproximado: `(2 * N^3) / (tiempo_wall * 1e9)`
- Speedup: `T_secuencial / T_paralelo`
- Eficiencia: `Speedup / P * 100%`

Ejemplo de interpretación:
```
Speedup (wall): 6.10x
Eficiencia (wall): 76.25% (8 hilos)
```

## 🤔 Hilos vs Procesos

| Aspecto | Hilos (pthreads) | Procesos (fork) |
|---------|------------------|-----------------|
| Memoria | Compartida implícita | Requiere `mmap`/IPC |
| Overhead creación | Bajo | Mayor |
| Sincronización | Necesaria para recursos compartidos | Menos necesidad si se divide memoria sin escribir zonas comunes |
| Medición de tiempo usuario | Acumula bien en `rusage` del proceso | Necesitas sumar tiempos de hijos externamente |

Para tamaños grandes, los procesos pueden ser un poco más lentos al inicio, pero el cómputo domina y el speedup debe acercarse al de pthreads si el reparto es similar.

## 🔍 División de Trabajo

Ambas versiones paralelas dividen el trabajo por filas. Si `N` no es múltiplo de `P`, las primeras filas reciben una más (`distribución balanceada`).

## 🛠 Extensiones Futuras (Ideas)
- Bloqueo / tiling para mejor uso de caché.
- Transponer B para acceso más contiguo.
- Uso de SIMD (intrinsics) y `-march=native`.
- OpenMP para comparar facilidad de implementación.

---
