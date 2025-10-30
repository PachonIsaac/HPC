# HPCReto2 - OpenMP Monte Carlo PI Estimation

## 📋 Descripción del Proyecto

Implementación de algoritmos Monte Carlo (Needles/Buffon y Dartboard) para el cálculo de PI utilizando paralelización con **OpenMP**. El proyecto incluye análisis de rendimiento, profiling, optimizaciones de CPU y memoria, y comparación con implementaciones previas (pthreads y fork).

### Algoritmos Implementados

1. **Buffon's Needle (Monte Carlo Needles)**
   - Simula el lanzamiento de agujas sobre líneas paralelas
   - Estima PI basándose en la probabilidad de que las agujas crucen las líneas

2. **Monte Carlo Dartboard**
   - Simula lanzamiento de dardos en un cuadrado con círculo inscrito
   - Estima PI usando la relación área círculo/cuadrado

## 📂 Estructura del Proyecto

```
HPCReto2/
├── Makefile                 # Sistema de compilación
├── README.md               # Este archivo
├── src/                    # Código fuente
│   ├── pi_needles_serial.c              # Needles serial (baseline)
│   ├── pi_dartboard_serial.c            # Dartboard serial (baseline)
│   ├── pi_needles_omp_basic.c           # Needles OpenMP básico
│   ├── pi_dartboard_omp_basic.c         # Dartboard OpenMP básico
│   ├── pi_needles_omp_static.c          # Needles con scheduling estático
│   ├── pi_needles_omp_dynamic.c         # Needles con scheduling dinámico
│   ├── pi_needles_omp_guided.c          # Needles con scheduling guiado
│   ├── pi_dartboard_omp_static.c        # Dartboard con scheduling estático
│   ├── pi_dartboard_omp_dynamic.c       # Dartboard con scheduling dinámico
│   ├── pi_dartboard_omp_guided.c        # Dartboard con scheduling guiado
│   ├── pi_needles_omp_optimized.c       # Needles optimizado (CPU+Mem)
│   └── pi_dartboard_omp_optimized.c     # Dartboard optimizado (CPU+Mem)
├── bin/                    # Ejecutables compilados
├── scripts/                # Scripts de automatización
│   ├── run_benchmarks.sh   # Ejecuta benchmarks completos
│   ├── run_profiling.sh    # Ejecuta profiling (gprof, perf)
│   └── compare_reto1.sh    # Compara con resultados Reto1
├── results/                # Resultados de benchmarks y profiling
│   ├── benchmarks.csv      # Datos de tiempo de ejecución
│   ├── speedup.csv         # Datos de speedup y eficiencia
│   └── profiling/          # Reportes de profiling
└── docs/                   # Análisis y documentación
    ├── analisis_paralelizacion.md
    ├── optimizaciones.md
    └── comparacion_reto1.md
```

## 🛠️ Compilación

### Requisitos
- GCC con soporte OpenMP
- Make
- Linux/macOS/Unix

### Compilar todo
```bash
make all
```

### Compilar por categorías
```bash
make serial      # Solo versiones seriales
make omp_basic   # Solo OpenMP básico
make omp_sched   # Solo diferentes scheduling
make omp_opt     # Solo optimizadas
make profiling   # Versiones para profiling
```

### Compilar individual
```bash
make pi_needles_omp_basic
make pi_dartboard_omp_optimized
```

### Ver ayuda
```bash
make help
```

### Limpiar
```bash
make clean       # Limpia binarios
make clean-all   # Limpia todo incluyendo resultados
```

## ▶️ Ejecución

### Ejecución manual
```bash
# Versiones seriales
./bin/pi_needles_serial 1000000
./bin/pi_dartboard_serial 1000000

# Versiones OpenMP (controlar threads con OMP_NUM_THREADS)
export OMP_NUM_THREADS=4
./bin/pi_needles_omp_basic 1000000
./bin/pi_dartboard_omp_optimized 10000000

# Diferentes números de threads
for threads in 1 2 4 8 16; do
    export OMP_NUM_THREADS=$threads
    ./bin/pi_needles_omp_basic 1000000
done
```

### Ejecución con scripts
```bash
# Benchmarks completos (múltiples tamaños, múltiples threads)
cd scripts
./run_benchmarks.sh

# Profiling
./run_profiling.sh

# Comparación con Reto1
./compare_reto1.sh
```

## 📊 Benchmarking

Los benchmarks evaluarán:
- **Speedup**: Aceleración respecto a versión serial
- **Eficiencia**: Qué tan bien se aprovechan los cores
- **Escalabilidad**: Comportamiento con diferentes números de threads
- **Precisión**: Exactitud del cálculo de PI
- **Diferentes tamaños de problema**: 10^6, 10^7, 10^8, 10^9 iteraciones

## 🔍 Profiling

Herramientas utilizadas:
- **gprof**: Análisis de tiempo por función
- **perf**: Contadores de rendimiento de CPU
- **valgrind/cachegrind**: Análisis de cache y memoria

## 🚀 Optimizaciones Aplicadas

### Nivel de Compilador
- `-O2`, `-O3`: Optimizaciones generales
- `-march=native`: Optimizaciones específicas del CPU
- `-funroll-loops`: Desenrollado de bucles

### Nivel de OpenMP
- Diferentes scheduling policies (static, dynamic, guided)
- Ajuste de chunk size
- Minimización de overhead de sincronización
- Reducción de false sharing

### Nivel de Algoritmo
- Generadores de números aleatorios thread-safe
- Reducción de operaciones matemáticas costosas
- Localidad de datos
- Alineación de memoria

## 📈 Resultados Esperados

Los resultados se guardarán en `results/`:
- `benchmarks.csv`: Tiempos de ejecución
- `speedup.csv`: Speedup y eficiencia calculados
- `profiling/`: Reportes detallados de profiling

## 🔄 Comparación con Reto1

Se compararán los resultados con las implementaciones anteriores:
- **Pthreads**: Control manual de threads
- **Fork/Processes**: Paralelismo mediante procesos
- **OpenMP**: Directivas de alto nivel

Aspectos a comparar:
- Facilidad de implementación
- Rendimiento y escalabilidad
- Overhead de paralelización
- Portabilidad

## 📚 Referencias

- **OpenMP Specifications**: https://www.openmp.org/specifications/
- **Método Monte Carlo**: Simulación estocástica
- **Buffon's Needle**: Problema clásico de probabilidad geométrica

## 👥 Autores

- Isaac Pachon

## 📅 Fecha

Octubre 2025 - HPC Course

---

**Nota**: Este proyecto es parte del curso de High Performance Computing (HPC).
