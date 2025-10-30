# Análisis de Paralelización - Algoritmos Monte Carlo

## 📊 Introducción

Este documento analiza las opciones de paralelización con OpenMP para los algoritmos Monte Carlo de estimación de PI: **Buffon's Needle** y **Dartboard**.

---

## 🎯 Algoritmo 1: Buffon's Needle (Monte Carlo Needles)

### Descripción del Algoritmo Serial
```c
for (long i = 0; i < needles; i++) {
    double x = ((double)rand() / RAND_MAX) * (DIST / 2);
    double theta = ((double)rand() / RAND_MAX) * M_PI;
    double reach = (LENGTH / 2) * sin(theta);
    if (x <= reach) hits++;
}
```

### Análisis de Dependencias

#### ✅ **Características Paralelizables**
1. **Iteraciones independientes**: Cada lanzamiento de aguja es independiente
2. **No hay dependencias de datos**: Cada iteración trabaja con variables locales
3. **Operación de reducción simple**: Solo necesitamos sumar `hits`

#### ⚠️ **Desafíos**
1. **Generador aleatorio thread-unsafe**: `rand()` no es thread-safe
2. **Variable compartida `hits`**: Requiere sincronización
3. **Operaciones matemáticas costosas**: `sin()` es computacionalmente cara

### Estrategias de Paralelización con OpenMP

#### **Opción 1: Parallel For Básico**
```c
#pragma omp parallel for reduction(+:hits)
for (long i = 0; i < needles; i++) {
    // Usar rand_r() con semilla thread-local
    double x = ((double)rand_r(&seed) / RAND_MAX) * (DIST / 2);
    double theta = ((double)rand_r(&seed) / RAND_MAX) * M_PI;
    double reach = (LENGTH / 2) * sin(theta);
    if (x <= reach) hits++;
}
```

**Ventajas**:
- Simple y directo
- `reduction(+:hits)` maneja la sincronización automáticamente
- Overhead mínimo de paralelización

**Consideraciones**:
- Necesita generador aleatorio thread-safe (`rand_r()`)
- Cada thread necesita su propia semilla

#### **Opción 2: Diferentes Scheduling Policies**

**Static Scheduling** (por defecto):
```c
#pragma omp parallel for schedule(static) reduction(+:hits)
```
- Distribuye iteraciones en chunks de tamaño fijo
- Bajo overhead
- Mejor para carga balanceada

**Dynamic Scheduling**:
```c
#pragma omp parallel for schedule(dynamic, 1000) reduction(+:hits)
```
- Asigna chunks dinámicamente
- Mayor overhead
- Útil si hay variabilidad en tiempo de ejecución

**Guided Scheduling**:
```c
#pragma omp parallel for schedule(guided) reduction(+:hits)
```
- Chunks adaptativos (grandes al inicio, pequeños al final)
- Compromiso entre static y dynamic

#### **Opción 3: Versión Optimizada**

```c
#pragma omp parallel reduction(+:hits)
{
    unsigned int seed = omp_get_thread_num() + time(NULL);
    
    #pragma omp for schedule(static)
    for (long i = 0; i < needles; i++) {
        double x = ((double)rand_r(&seed) / RAND_MAX) * (DIST / 2);
        double theta = ((double)rand_r(&seed) / RAND_MAX) * M_PI;
        double reach = (LENGTH / 2) * sin(theta);
        if (x <= reach) hits++;
    }
}
```

**Optimizaciones adicionales**:
1. **Precalcular constantes**: `LENGTH/2`, `DIST/2`
2. **Lookup tables para sin()**: Si es viable
3. **Usar generadores más rápidos**: `drand48_r()` o generadores custom
4. **Padding para evitar false sharing**

---

## 🎯 Algoritmo 2: Dartboard (Monte Carlo Circle)

### Descripción del Algoritmo Serial
```c
for (long i = 0; i < darts; i++) {
    double x = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
    double y = ((double)rand() / RAND_MAX) * 2.0 - 1.0;
    if (x * x + y * y <= 1.0) hits++;
}
```

### Análisis de Dependencias

#### ✅ **Características Paralelizables**
1. **Iteraciones completamente independientes**
2. **Operaciones matemáticas simples**: Solo multiplicación y suma
3. **Sin funciones transcendentales**: Más rápido que Needles

#### ⚠️ **Desafíos**
1. **Mismo problema con `rand()`**: No thread-safe
2. **Variable compartida `hits`**: Reducción necesaria

### Estrategias de Paralelización con OpenMP

#### **Opción 1: Parallel For Básico**
```c
#pragma omp parallel for reduction(+:hits)
for (long i = 0; i < darts; i++) {
    unsigned int seed = i + omp_get_thread_num();
    double x = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
    double y = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
    if (x * x + y * y <= 1.0) hits++;
}
```

#### **Opción 2: Versión Optimizada con Menos Operaciones**
```c
#pragma omp parallel reduction(+:hits)
{
    unsigned int seed = omp_get_thread_num() + time(NULL);
    
    #pragma omp for schedule(static)
    for (long i = 0; i < darts; i++) {
        // Evitar multiplicación por 2.0 - 1.0
        double x = ((double)rand_r(&seed) / RAND_MAX);
        double y = ((double)rand_r(&seed) / RAND_MAX);
        x = x + x - 1.0;  // Más rápido que * 2.0
        y = y + y - 1.0;
        
        // Comparación directa sin sqrt
        if (x * x + y * y <= 1.0) hits++;
    }
}
```

#### **Opción 3: Minimizar False Sharing**
```c
#pragma omp parallel
{
    int local_hits = 0;  // Variable local por thread
    unsigned int seed = omp_get_thread_num() + time(NULL);
    
    #pragma omp for schedule(static) nowait
    for (long i = 0; i < darts; i++) {
        double x = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        double y = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;
        if (x * x + y * y <= 1.0) local_hits++;
    }
    
    #pragma omp atomic
    hits += local_hits;  // Sincronización al final
}
```

---

## 📊 Comparación: Needles vs Dartboard

| Aspecto | Needles | Dartboard |
|---------|---------|-----------|
| **Complejidad por iteración** | Alta (sin, división) | Baja (multiplicación) |
| **Convergencia a PI** | Más lenta | Más rápida |
| **Paralelización** | Igual de directa | Igual de directa |
| **Overhead esperado** | Mayor (sin costosa) | Menor (ops simples) |
| **Speedup esperado** | Bueno (trabajo >> overhead) | Excelente (trabajo ligero pero muchas iter) |

---

## 🔍 Aspectos Importantes de OpenMP

### Variables y Cláusulas

#### **Private vs Shared**
```c
#pragma omp parallel for private(x, y, theta, reach) shared(needles, hits)
```
- `private`: Cada thread tiene su copia (x, y, theta, reach)
- `shared`: Todos los threads acceden a la misma (needles, hits)
- Por defecto: variables en el scope son compartidas

#### **Reduction**
```c
#pragma omp parallel for reduction(+:hits)
```
- Crea copias privadas de `hits`
- Cada thread acumula en su copia
- Al final, suma todas las copias atómicamente
- **Ideal para nuestro caso**

#### **Scheduling**
- `schedule(static)`: Mejor performance si iteraciones uniformes
- `schedule(static, chunk_size)`: Control manual del chunk
- `schedule(dynamic)`: Balanceo de carga dinámico
- `schedule(guided)`: Adaptativo (recomendado probar)

### Evitar False Sharing

**Problema**: Múltiples threads modificando variables en la misma línea de cache

**Solución**:
```c
struct AlignedCounter {
    long value;
    char padding[64 - sizeof(long)];  // Alinear a cache line
} __attribute__((aligned(64)));
```

### Inicialización de Semillas

**Mala práctica**:
```c
unsigned int seed = time(NULL);  // Todos los threads igual semilla
```

**Buena práctica**:
```c
unsigned int seed = omp_get_thread_num() + time(NULL) * (omp_get_thread_num() + 1);
```

---

## 🎯 Plan de Implementación

### Fase 1: Implementaciones Básicas
1. ✅ `pi_needles_serial.c` - Baseline
2. ✅ `pi_dartboard_serial.c` - Baseline
3. `pi_needles_omp_basic.c` - OpenMP básico con reduction
4. `pi_dartboard_omp_basic.c` - OpenMP básico con reduction

### Fase 2: Exploración de Scheduling
5. `pi_needles_omp_static.c` - schedule(static)
6. `pi_needles_omp_dynamic.c` - schedule(dynamic)
7. `pi_needles_omp_guided.c` - schedule(guided)
8. Similar para dartboard (3 versiones)

### Fase 3: Optimizaciones
9. `pi_needles_omp_optimized.c` - Todas las optimizaciones
10. `pi_dartboard_omp_optimized.c` - Todas las optimizaciones

### Optimizaciones a Incluir:
- ✅ Generadores aleatorios thread-safe
- ✅ Evitar false sharing
- ✅ Precálculo de constantes
- ✅ Minimizar llamadas a funciones caras
- ✅ Alineación de datos
- ✅ Flags de compilación agresivos (-O3, -march=native)

---

## 📈 Métricas de Evaluación

### Speedup
$$S_p = \frac{T_{serial}}{T_{parallel}}$$

### Eficiencia
$$E_p = \frac{S_p}{p} = \frac{T_{serial}}{p \times T_{parallel}}$$

### Overhead
$$O = T_{parallel} - \frac{T_{serial}}{p}$$

### Strong Scaling
- Problema de tamaño fijo
- Aumentar número de procesadores
- Esperado: Speedup lineal (ideal) o sub-lineal (realista)

### Weak Scaling
- Aumentar problema proporcionalmente con procesadores
- Esperado: Tiempo constante (ideal)

---

## 🔬 Experimentos Sugeridos

1. **Variación de tamaño**: 10^6, 10^7, 10^8, 10^9 iteraciones
2. **Variación de threads**: 1, 2, 4, 8, 16 threads
3. **Scheduling policies**: static vs dynamic vs guided
4. **Chunk sizes**: Para dynamic/guided
5. **Comparación con Reto1**: pthreads vs fork vs OpenMP

---

## ✅ Conclusiones Preliminares

### Ventajas de OpenMP para estos algoritmos:
1. **Simplicidad**: Directivas simples vs manejo manual de threads
2. **Portabilidad**: Estándar multiplataforma
3. **Eficiencia**: Overhead bajo para trabajo computacional
4. **Flexibilidad**: Fácil experimentar con diferentes estrategias

### Desafíos:
1. **Random numbers**: Necesita manejo especial thread-safety
2. **Tune scheduling**: Requiere experimentación
3. **Análisis de performance**: Profiling necesario

### Resultados Esperados:
- **Speedup cercano a lineal** para ambos algoritmos (trabajo >> overhead)
- **Dartboard ligeramente mejor** que Needles (operaciones más simples)
- **Static scheduling mejor** para estos casos (iteraciones uniformes)
- **OpenMP comparable o mejor** que pthreads (menos overhead manual)

---
