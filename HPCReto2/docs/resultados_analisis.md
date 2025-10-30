# Resultados y Análisis - HPCReto2

## 📊 Resumen Ejecutivo

Este documento presenta los resultados completos del benchmarking de las implementaciones OpenMP de los algoritmos Monte Carlo para estimación de PI (Buffon's Needle y Dartboard).

**Configuración de pruebas:**
- **Tamaños**: 1M y 10M iteraciones
- **Threads**: 2, 4, 8 threads
- **Ejecuciones**: 3 runs por configuración
- **Plataforma**: macOS ARM64 (Apple Silicon)
- **Compilador**: Apple Clang 17.0.0 con libomp

---

## 🏆 Mejores Configuraciones

### Dartboard

**Mejor Speedup:**
- Versión: `OMP_Optimized`
- Threads: 8
- Speedup: **4.85x**
- Eficiencia: 60.7%
- Iteraciones: 10M

**Mejor Eficiencia:**
- Versión: `OMP_Guided`
- Threads: 2
- Speedup: 2.03x
- Eficiencia: **101.5%**
- Iteraciones: 10M

### Needles (Buffon)

**Mejor Speedup:**
- Versión: `OMP_Guided`
- Threads: 8
- Speedup: **5.88x**
- Eficiencia: 73.5%
- Iteraciones: 10M

**Mejor Eficiencia:**
- Versión: `OMP_Dynamic`
- Threads: 2
- Speedup: 2.73x
- Eficiencia: **136.5%** (superlinear!)
- Iteraciones: 10M

---

## 📈 Análisis Detallado por Algoritmo

### 1. Buffon's Needle (Needles)

#### Rendimiento General
El algoritmo Needles muestra **excelente escalabilidad** gracias a su carga computacional por iteración (función `sin()`).

#### Resultados por Configuración (10M iteraciones)

| Version | Threads | Tiempo (s) | Speedup | Eficiencia |
|---------|---------|------------|---------|------------|
| Serial | 1 | 0.2422 | 1.00x | 100.0% |
| OMP_Basic | 2 | 0.0917 | 2.64x | 132.0% |
| OMP_Basic | 4 | 0.0648 | 3.74x | 93.4% |
| OMP_Basic | 8 | 0.0480 | 5.04x | 63.0% |
| **OMP_Dynamic** | 2 | 0.0887 | **2.73x** | **136.5%** |
| **OMP_Dynamic** | 4 | 0.0497 | **4.87x** | **121.8%** |
| **OMP_Dynamic** | 8 | 0.0414 | **5.84x** | **73.0%** |
| **OMP_Guided** | 2 | 0.0887 | **2.73x** | **136.5%** |
| **OMP_Guided** | 4 | 0.0493 | **4.91x** | **122.7%** |
| **OMP_Guided** | 8 | 0.0412 | **5.88x** | **73.5%** |
| OMP_Optimized | 2 | 0.1302 | 1.86x | 93.0% |
| OMP_Optimized | 4 | 0.0689 | 3.51x | 87.8% |
| OMP_Optimized | 8 | 0.0454 | 5.34x | 66.7% |
| OMP_Static | 2 | 0.0919 | 2.64x | 131.8% |
| OMP_Static | 4 | 0.0557 | 4.34x | 108.6% |
| OMP_Static | 8 | 0.0481 | 5.03x | 62.9% |

#### Observaciones Clave

✅ **Speedup superlinear en 2 threads** (>2.6x):
- Probablemente debido a mejoras en cache locality
- Cada thread tiene datos más compactos en L1/L2 cache

✅ **Dynamic y Guided scheduling superan a Static**:
- Dynamic: 5.84x vs Static: 5.03x (8 threads)
- Mejor balance de carga a pesar del overhead

⚠️ **Versión "optimizada" más lenta**:
- Overhead de gestión de contadores alineados
- Reducción manual vs reduction automática
- Trade-off no favorable para este problema

---

### 2. Monte Carlo Dartboard

#### Rendimiento General
Dartboard tiene operaciones más simples (sin funciones transcendentales), pero aún muestra **buena escalabilidad**.

#### Resultados por Configuración (10M iteraciones)

| Version | Threads | Tiempo (s) | Speedup | Eficiencia |
|---------|---------|------------|---------|------------|
| Serial | 1 | 0.1313 | 1.00x | 100.0% |
| OMP_Basic | 2 | 0.0654 | 2.01x | 100.5% |
| OMP_Basic | 4 | 0.0408 | 3.22x | 80.6% |
| OMP_Basic | 8 | 0.0328 | 4.00x | 50.0% |
| OMP_Dynamic | 2 | 0.0650 | 2.02x | 101.2% |
| **OMP_Dynamic** | 4 | 0.0351 | **3.74x** | **93.5%** |
| **OMP_Dynamic** | 8 | 0.0279 | **4.70x** | **58.8%** |
| OMP_Guided | 2 | 0.0647 | 2.03x | 101.5% |
| OMP_Guided | 4 | 0.0353 | 3.72x | 93.0% |
| OMP_Guided | 8 | 0.0282 | 4.67x | 58.3% |
| OMP_Optimized | 2 | 0.0833 | 1.58x | 78.9% |
| OMP_Optimized | 4 | 0.0417 | 3.15x | 78.7% |
| **OMP_Optimized** | 8 | 0.0271 | **4.85x** | **60.7%** |
| OMP_Static | 2 | 0.0653 | 2.01x | 100.7% |
| OMP_Static | 4 | 0.0404 | 3.25x | 81.3% |
| OMP_Static | 8 | 0.0356 | 3.69x | 46.2% |

#### Observaciones Clave

✅ **Escalabilidad casi lineal hasta 4 threads**:
- 2 threads: ~2.0x
- 4 threads: ~3.7x
- 8 threads: ~4.7x (mejor configuración)

✅ **Dynamic scheduling gana**:
- 4.70x vs 3.69x (Static) con 8 threads
- 27% mejor rendimiento

✅ **Versión optimizada efectiva con 8 threads**:
- Mejor speedup absoluto: 4.85x
- Evitar false sharing es crítico con más threads

---

## 🔬 Comparación de Scheduling Policies

### Needles

| Threads | Static | Dynamic | Guided |
|---------|--------|---------|--------|
| 2 | 2.64x | **2.73x** | **2.73x** |
| 4 | 4.34x | **4.87x** | **4.91x** |
| 8 | 5.03x | **5.84x** | **5.88x** |

**Winner**: **Guided scheduling** (mejor speedup global)

### Dartboard

| Threads | Static | Dynamic | Guided |
|---------|--------|---------|--------|
| 2 | 2.01x | 2.02x | **2.03x** |
| 4 | 3.25x | **3.74x** | 3.72x |
| 8 | 3.69x | **4.70x** | 4.67x |

**Winner**: **Dynamic scheduling** (mejor speedup con más threads)

---

## 💡 Insights y Conclusiones

### 1. **Scheduling Policy Importa**

- **Static**: Rápido para configuración, pero peor balance de carga
- **Dynamic**: Mejor para balancear carga, overhead moderado
- **Guided**: Mejor compromiso (chunks adaptativos)

**Recomendación**: Usar **Guided** por defecto, **Dynamic** si hay desbalance

### 2. **Speedup Superlineal Explicado**

Observamos eficiencias >100% en varias configuraciones:
- **Needles con 2 threads**: 136.5% eficiencia
- **Causa**: Mejor utilización de cache L1/L2
- **Explicación**: Datos de cada thread caben en cache privada

### 3. **Needles vs Dartboard**

| Aspecto | Needles | Dartboard |
|---------|---------|-----------|
| Mejor Speedup | 5.88x (8 threads) | 4.85x (8 threads) |
| Escalabilidad | Excelente | Muy buena |
| Eficiencia pico | 136.5% | 101.5% |
| Sensibilidad a scheduling | Alta | Moderada |

**Needles escala mejor** debido a mayor trabajo por iteración (sin() es cara).

### 4. **Optimizaciones Manuales**

La versión "optimizada" con manejo manual de false sharing:
- ❌ **No siempre es mejor**
- ✅ **Efectiva con 8 threads** en Dartboard (4.85x)
- ⚠️ **Overhead supera beneficio** en problemas pequeños

**Lección**: `reduction` de OpenMP está muy optimizada, difícil de superar manualmente.

### 5. **Escalabilidad Fuerte (Strong Scaling)**

Ambos algoritmos muestran **strong scaling sub-lineal** típico:

**Needles (10M iteraciones)**:
- 2 threads: 2.73x (136% eficiencia) ⭐
- 4 threads: 4.91x (123% eficiencia) ⭐
- 8 threads: 5.88x (74% eficiencia)

**Dartboard (10M iteraciones)**:
- 2 threads: 2.03x (102% eficiencia)
- 4 threads: 3.74x (94% eficiencia)
- 8 threads: 4.85x (61% eficiencia)

**Ley de Amdahl** se manifiesta: overhead de paralelización aumenta con más threads.

---

## 🎯 Recomendaciones Prácticas

### Para Needles:
1. **Mejor configuración general**: `OMP_Guided` con 8 threads (5.88x)
2. **Mejor eficiencia**: `OMP_Dynamic` con 2 threads (136.5%)
3. **Sweet spot**: 4 threads con Guided (4.91x, 123% eficiencia)

### Para Dartboard:
1. **Mejor configuración general**: `OMP_Optimized` con 8 threads (4.85x)
2. **Mejor eficiencia**: `OMP_Guided` con 2 threads (101.5%)
3. **Sweet spot**: 4 threads con Dynamic (3.74x, 94% eficiencia)

### Elección de Scheduling:
- **Cargas uniformes** (nuestro caso): Guided > Dynamic > Static
- **Cargas variables**: Dynamic
- **Máximo performance**: Experimentar con ambos

---

## 📊 Archivos Generados

### Datos
- `results/benchmarks.csv` - Datos crudos (192 mediciones)
- `results/statistics.csv` - Estadísticas agregadas
- `results/speedup_analysis.csv` - Análisis de speedup/eficiencia

### Gráficas
- `results/plots/needles_speedup_threads.png`
- `results/plots/needles_efficiency_threads.png`
- `results/plots/needles_time_comparison.png`
- `results/plots/dartboard_speedup_threads.png`
- `results/plots/dartboard_efficiency_threads.png`
- `results/plots/dartboard_time_comparison.png`
- `results/plots/scheduling_comparison.png`
- `results/plots/scalability_analysis.png`

---

## 🔄 Próximos Pasos

1. ✅ **Benchmarking completado**
2. ✅ **Análisis de datos completado**
3. ✅ **Visualizaciones generadas**
4. ⏳ **Profiling detallado** (gprof, perf, cachegrind)
5. ⏳ **Comparación con Reto1** (pthreads vs fork vs OpenMP)
6. ⏳ **Análisis de causas de rendimiento**

---

## 📚 Referencias

- OpenMP Specifications 5.0
- "Using OpenMP" - Barbara Chapman et al.
- "The Art of Multiprocessor Programming" - Herlihy & Shavit

---

**Fecha**: 29 de Octubre de 2025  
**Autor**: Isaac Pachon  
**Curso**: High Performance Computing (HPC)
