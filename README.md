# Multiplicación de Matrices - High Performance Computing (HPC)

Este repositorio contiene implementaciones de multiplicación de matrices orientadas a benchmarking en entornos HPC: versión secuencial, versiones paralelas con POSIX Threads (incluida una variante optimizada) y una implementación con procesos (`fork` + `mmap`). Además incluye un ejecutable comparativo unificado (hilos vs procesos), un modo de salida CSV reproducible y scripts para automatizar benchmarks y generar tablas agregadas.

## 🚀 Características

- **Versión Secuencial**: Implementación clásica O(n³) (archivo independiente)
- **Versión Paralela con Hilos (pthreads)**: División de filas con reparto balanceado
- **Versión Paralela con Procesos (fork + mmap)**: Implementación con memoria compartida
- **Versión Pthreads Optimizada**: Variantes con potencial mejor uso de caché / flags CPU
- **Ejecutable Comparativo Unificado (`matrix_mult_all`)**: Compara únicamente Hilos vs Procesos (la versión secuencial fue removida de este binario para reducir tiempo de ejecución de benchmarks masivos)
- **Generación Aleatoria Reproducible**: Semillas configurables para A y B
- **Medición de Rendimiento**: En el comparativo: solo tiempo de pared (wall). En ejecutables individuales se puede extender a GFLOPS/s si se desea calcular externamente.
- **Modo CSV**: Salida estructurada para pipelines de análisis (`--csv`, `--csv-header`, `--run`)
- **Script de Benchmarks Masivos**: Genera automáticamente `benchmarks.csv` para múltiples tamaños y números de trabajadores
- **Generación Automática de Tablas**: Pivotes (por hilos, por procesos, por tamaño y ratios) y agregación con fila PROMEDIO
- **Verificación de Correctitud**: Comparación entre resultados hilos vs procesos en el binario unificado

## 📁 Archivos del Proyecto

### Código Fuente
- `matrix_multiplication.c` - Versión secuencial básica
- `matrix_multiplication_pthread.c` - Versión paralela con pthreads
- `matrix_multiplication_pthread_optimized.c` - Versión paralela optimizada (variantes experimentales)
- `matrix_multiplication_all.c` - Comparación Hilos vs Procesos + modo CSV
- `matrix_time_analysis.c` - (Opcional) exploración de tiempos adicionales

### Scripts y Utilidades
- `run_benchmarks.sh` - Ejecuta baterías de pruebas y genera `benchmarks.csv`
- `generate_tables.py` - Crea pivotes y tablas agregadas a partir del CSV
- `test_matrix.sh` - Pruebas básicas de correctitud (comparaciones simples)
- `Makefile` - Automatización de compilación
- `README.md` - Documentación
- `.gitignore` - Exclusiones de control de versiones

## 🔧 Compilación

### Opción 1: Usando Makefile (Recomendado)

```bash
make                          # matrix_mult (secuencial)
make matrix_mult_pthread      # Versión paralela (hilos)
make matrix_mult_pthread_opt  # Versión paralela optimizada
make matrix_mult_all          # Comparativo hilos vs procesos + modo CSV
make matrix_time_analysis     # (Opcional) análisis extendido
make all                      # Compilar todas las anteriores
```

Nota: El comparativo ya NO ejecuta la versión secuencial (fue retirada para acelerar campañas masivas). Si necesitas un baseline secuencial, ejecútalo por separado con `./matrix_mult`.

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

### Versión Comparativa Unificada (Hilos vs Procesos)
```bash
./matrix_mult_all <tamaño> [trabajadores] [semilla_A] [semilla_B]
# Ejemplos
./matrix_mult_all 1024
./matrix_mult_all 1024 8
./matrix_mult_all 1024 8 123 456
```

Notas:
- Si no se indica cantidad de trabajadores, se usa el número de CPUs detectadas.
- Las semillas garantizan reproducibilidad entre ejecuciones (misma A y B para hilos y procesos).

#### Modo CSV
El ejecutable `matrix_mult_all` soporta un modo estructurado para recolectar datos:

Flags:
- `--csv-header`  Imprime la cabecera: `run,n,workers,thr_wall_s,proc_wall_s,win`
- `--csv`         Imprime solo la fila de datos correspondiente a la ejecución (sin texto adicional)
- `--run=N`       Etiqueta numérica (entero) para la columna `run` (repetición / id de corrida)

Ejemplo aislado:
```bash
./matrix_mult_all 1024 8 --csv-header
./matrix_mult_all 1024 8 --csv --run=1
./matrix_mult_all 1024 8 --csv --run=2
```

Salida típica de una fila CSV:
```
1,1024,8,0.842311,0.967552,threads
```
Donde `win` toma valores `threads`, `processes` o `tie` según menor tiempo de pared.

## 🧪 Pruebas y Benchmarks

```bash
make test             # Pruebas básicas secuenciales
./test_matrix.sh      # Comparaciones simples (puede invocar hilos y procesos)
```

Para una comparación directa puntual:
```bash
./matrix_mult_all 1600 8 123 456
```

### Benchmark Automático Masivo
El script `run_benchmarks.sh` recorre un conjunto de tamaños y números de trabajadores, repite varias veces y produce un único archivo `benchmarks.csv` acumulado.

Estructura de columnas:
```
run,n,workers,thr_wall_s,proc_wall_s,win
```

Ejecutar:
```bash
bash run_benchmarks.sh
```

Al finalizar tendrás un archivo `benchmarks.csv` listo para análisis o para alimentar el generador de tablas.

Si deseas ajustar parámetros (tamaños, workers, repeticiones) edita las variables `SIZES`, `WORKERS` y `REPS` dentro del script.

### Generación de Tablas Agregadas
El script `generate_tables.py` (requiere Python 3 y `pandas`) crea directorios con pivotes y un índice.

Instalar dependencia (si no tienes pandas):
```bash
python3 -m pip install --user pandas
```

Ejecutar:
```bash
python3 generate_tables.py
```

Se generan (ejemplo):
- `tables_threads/`    Tablas pivot con `thr_wall_s` por tamaño (filas) y workers (columnas)
- `tables_processes/`  Igual para `proc_wall_s`
- `tables_ratio/`      Razón `proc_wall_s / thr_wall_s` (<1 favorece procesos, >1 favorece hilos)
- `tables_by_size/`    Pivotes invertidos (workers en filas) por cada tamaño
- `tables_index.md`    Índice de navegación

Cada tabla incluye una fila final `PROMEDIO` (media aritmética sobre las filas de datos).

## 📊 Métricas y Análisis

En el modo comparativo (`matrix_mult_all`) actualmente SOLO se registra tiempo de pared para cada estrategia. Esto simplifica y acelera campañas grandes. A partir de esos tiempos puedes derivar externamente:

- Speedup (si tomas un baseline secuencial medido aparte): `T_seq / T_parallel`
- GFLOPS aproximado (si deseas calcularlo): `(2 * N^3) / (wall_seconds * 1e9)`
- Eficiencia: `Speedup / P`

La columna `win` en el CSV permite conteos rápidos sobre cuántas veces gana una estrategia por configuración.

## 🤔 Hilos vs Procesos

| Aspecto | Hilos (pthreads) | Procesos (fork) |
|---------|------------------|-----------------|
| Memoria | Compartida implícita | Requiere `mmap`/IPC |
| Overhead creación | Bajo | Mayor |
| Sincronización | Necesaria para recursos compartidos | Menos necesidad si se divide memoria sin escribir zonas comunes |
| Medición de tiempo usuario | Acumula bien en `rusage` del proceso | Necesitas sumar tiempos de hijos externamente |

Observaciones típicas:
- Overhead inicial de procesos puede penalizar tamaños pequeños.
- Para tamaños medianos/grandes el costo de cómputo domina y las brechas se reducen.
- Si la afinidad CPU y caché son favorables, los hilos suelen mantener ligera ventaja.

## 🔍 División de Trabajo

Ambas versiones paralelas dividen el conjunto de filas de A en porciones casi iguales. Si `N` no es múltiplo de `P`, algunas unidades reciben una fila extra (estrategia de reparto balanceado "round-robin compact").

## 🛠 Extensiones Futuras (Ideas)
- Bloqueo / tiling para mejor uso de caché
- Transponer B para acceso más contiguo
- SIMD (intrinsics) + `-march=native`
- OpenMP / MPI para comparar modelos
- Escritura incremental de resultados parciales (streams) en matrices muy grandes

---

## 📦 Flujo de Trabajo Sugerido
1. Compilar: `make all`
2. (Opcional) Validar correctitud puntual: `./matrix_mult_all 512 8 123 456`
3. Ejecutar benchmarks masivos: `bash run_benchmarks.sh`
4. Generar tablas: `python3 generate_tables.py`
5. Analizar / graficar (`benchmarks.csv` o tablas) en la herramienta de tu preferencia (Power BI, Python, etc.)

---

## ✅ Verificación de Correctitud
El binario comparativo compara internamente los resultados de hilos y procesos y aborta si detecta una discrepancia. Para validar contra la versión secuencial puedes ejecutar manualmente:
```bash
./matrix_mult 512 123 456 > seq.out
./matrix_mult_pthread 512 8 123 456 > thr.out
diff seq.out thr.out   # Debe ser vacío (ignorando cabeceras/tiempos si los rediriges selectivamente)
```

---

## 📄 Licencia
Uso académico / educativo. Ajusta o extiende según tus necesidades.

---
