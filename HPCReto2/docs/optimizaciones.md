# Optimizaciones Aplicadas - HPCReto2

## 📊 Resumen de Implementaciones

Se han creado **12 implementaciones diferentes** de los algoritmos Monte Carlo:
- 2 versiones seriales (baseline)
- 2 versiones OpenMP básicas
- 6 versiones con diferentes scheduling policies
- 2 versiones completamente optimizadas

---

## 🔧 Optimizaciones por Categoría

### 1. **Optimizaciones de Paralelización (OpenMP)**

#### Directivas Básicas
```c
#pragma omp parallel for reduction(+:hits) schedule(static)
```

**Beneficios**:
- `parallel for`: Automáticamente distribuye iteraciones
- `reduction(+:hits)`: Sincronización eficiente del contador
- Overhead mínimo de thread management

#### Scheduling Policies Exploradas

**a) Static Scheduling**
```c
schedule(static)
```
- División de trabajo en chunks de tamaño fijo
- Menor overhead
- **Mejor para iteraciones uniformes** (nuestro caso)
- Distribución predecible

**b) Dynamic Scheduling**
```c
schedule(dynamic, 1000)
```
- Chunks asignados dinámicamente
- Mayor overhead de sincronización
- Útil para carga desbalanceada
- Chunk size de 1000 iteraciones

**c) Guided Scheduling**
```c
schedule(guided)
```
- Chunks adaptativos (grandes → pequeños)
- Compromiso entre static y dynamic
- Buen balanceo de carga con overhead moderado

---

### 2. **Optimizaciones de Generación de Números Aleatorios**

#### Problema Original
```c
// ❌ rand() no es thread-safe
srand(time(NULL));
for (i = 0; i < n; i++) {
    x = rand();  // Race condition!
}
```

#### Solución Implementada
```c
// ✅ rand_r() con semilla por thread
unsigned int seed = (unsigned int)(i + omp_get_thread_num() * needles);
double x = ((double)rand_r(&seed) / RAND_MAX) * range;
```

**Beneficios**:
- Thread-safe
- Cada thread tiene su secuencia aleatoria
- Sin necesidad de locks

#### Versión Optimizada
```c
// Semilla única por thread (no por iteración)
unsigned int seed = (unsigned int)(time(NULL) + tid * 12345);

#pragma omp for
for (long i = 0; i < n; i++) {
    double x = rand_r(&seed) * rand_max_inv;  // Precalculada
}
```

**Mejoras**:
- Semilla solo una vez por thread (más eficiente)
- Precálculo de `1.0/RAND_MAX` evita división repetida

---

### 3. **Optimizaciones para Evitar False Sharing**

#### Problema
```c
// ❌ Múltiples threads modificando hits
long hits = 0;
#pragma omp parallel for
for (i = 0; i < n; i++) {
    if (condition) hits++;  // False sharing!
}
```

#### Solución: Contadores Locales Alineados
```c
// ✅ Estructura alineada a cache line (64 bytes)
typedef struct {
    long value;
    char padding[64 - sizeof(long)];
} aligned_long_t __attribute__((aligned(64)));

aligned_long_t *local_hits = calloc(num_threads, sizeof(aligned_long_t));

#pragma omp parallel
{
    int tid = omp_get_thread_num();
    long my_hits = 0;
    
    #pragma omp for nowait
    for (i = 0; i < n; i++) {
        if (condition) my_hits++;
    }
    
    local_hits[tid].value = my_hits;
}

// Reducción final sin contención
for (i = 0; i < num_threads; i++) {
    total_hits += local_hits[i].value;
}
```

**Beneficios**:
- Cada thread escribe en su propia cache line
- Elimina invalidaciones de cache
- Reducción solo al final (una vez)
- Uso de `nowait` para eliminar barrera innecesaria

---

### 4. **Optimizaciones de CPU**

#### A) Precálculo de Constantes
```c
// ❌ Antes: calcular en cada iteración
for (i = 0; i < n; i++) {
    double half_length = LENGTH / 2.0;  // Repetido millones de veces
    double x = ... * (LENGTH / 2.0);
}

// ✅ Después: calcular una vez
const double half_length = LENGTH / 2.0;
const double half_dist = DIST / 2.0;
const double rand_max_inv = 1.0 / RAND_MAX;

for (i = 0; i < n; i++) {
    double x = rand_r(&seed) * rand_max_inv * half_dist;
}
```

**Impacto**: Elimina millones de divisiones

#### B) Optimización de Operaciones Matemáticas

**Dartboard - Transformación de rango**
```c
// ❌ Antes: 2 multiplicaciones
double x = ((double)rand_r(&seed) / RAND_MAX) * 2.0 - 1.0;

// ✅ Después: 1 multiplicación + 1 suma
double x = ((double)rand_r(&seed) * rand_max_inv);
x = x + x - 1.0;  // Más rápido que x * 2.0
```

**Beneficio**: Suma es más rápida que multiplicación

#### C) Flags de Compilación
```makefile
CFLAGS_OMP_OPT = -O3 -march=native -funroll-loops
```

- **-O3**: Optimizaciones agresivas del compilador
- **-march=native**: Instrucciones específicas del CPU
- **-funroll-loops**: Desenrolla bucles pequeños

---

### 5. **Optimizaciones de Memoria**

#### A) Localidad de Datos
```c
// Todas las variables locales por thread
#pragma omp parallel
{
    long my_hits = 0;           // Stack local
    unsigned int seed = ...;     // Stack local
    
    for (...) {
        double x, y, theta;      // Registros o stack
        // Trabajo computacional
    }
}
```

**Beneficios**:
- Datos en cache L1 del core
- No accesos a memoria compartida en el loop
- Maximiza localidad temporal

#### B) Alineación de Estructuras
```c
__attribute__((aligned(64)))
```
- Alinea estructuras a boundaries de cache line
- Evita que una estructura cruce múltiples cache lines
- Accesos más eficientes

#### C) Reducción de Accesos a Memoria
```c
// ✅ Acumulación local, escritura una vez
long my_hits = 0;
for (i = 0; i < millions; i++) {
    if (condition) my_hits++;  // Solo incremento en registro
}
local_hits[tid].value = my_hits;  // Escritura única a memoria
```

---

## 📈 Resultados Esperados de Optimizaciones

### Speedup Esperado por Optimización

| Optimización | Speedup Esperado | Razón |
|--------------|------------------|-------|
| OpenMP básico (4 threads) | 3.0-3.5x | Overhead paralelización ~15% |
| Evitar false sharing | +10-20% | Menos invalidaciones cache |
| Precálculo constantes | +5-10% | Elimina divisiones |
| Optimizar operaciones | +5-15% | Menos instrucciones |
| Flags compilador | +10-20% | Vectorización, inlining |
| **Total combinado** | **4.0-4.5x** | Sinergias entre optimizaciones |

### Comparación de Scheduling

Para nuestros algoritmos (iteraciones uniformes):

| Scheduling | Performance Relativa | Overhead |
|------------|---------------------|----------|
| Static | 100% (baseline) | Mínimo |
| Guided | 98-99% | Bajo |
| Dynamic | 90-95% | Moderado |

**Conclusión**: Static debería ser el mejor para Monte Carlo

---

## 🔬 Análisis Específico por Algoritmo

### Needles (Buffon)

**Características**:
- Operación costosa: `sin(theta)`
- Trabajo por iteración: Alto
- Ratio computación/memoria: Favorable para paralelización

**Optimizaciones más efectivas**:
1. ✅ Paralelización (sin() es cara)
2. ✅ Precálculo de constantes
3. ✅ Evitar false sharing
4. Lookup table para sin() (no implementada)

**Limitaciones**:
- `sin()` no se puede optimizar mucho
- Depende de librería math

### Dartboard (Circle)

**Características**:
- Operaciones simples: multiplicación, suma
- Trabajo por iteración: Bajo
- Más iteraciones típicamente necesarias

**Optimizaciones más efectivas**:
1. ✅ Paralelización (overhead < beneficio)
2. ✅ Evitar false sharing (crítico para ops rápidas)
3. ✅ Optimizar operaciones (x+x vs x*2)
4. ✅ Flags de compilación (-O3 vectoriza mejor)

**Ventajas**:
- Sin funciones transcendentales
- Mejor candidato para vectorización SIMD
- Mayor speedup esperado

---

## 🎯 Mejoras Futuras Posibles

### No Implementadas (pero posibles)

1. **SIMD Vectorization Manual**
```c
#pragma omp simd
for (i = 0; i < n; i += 4) {
    // Procesar 4 elementos simultáneamente
}
```

2. **Generadores Aleatorios Más Rápidos**
- PCG (Permuted Congruential Generator)
- xorshift
- Menor calidad pero 2-3x más rápido

3. **Lookup Tables para sin()**
```c
double sin_table[1000];
// Pre-calcular valores comunes
```

4. **NUMA Awareness**
```c
#pragma omp parallel proc_bind(close)
```

5. **Prefetching**
```c
__builtin_prefetch(data, 0, 3);
```

---

## ✅ Conclusiones

### Optimizaciones Implementadas

✅ **Nivel de Paralelización**:
- OpenMP parallel for
- Reduction clauses
- 3 scheduling policies

✅ **Nivel de Algoritmo**:
- Thread-safe random generators
- Evitar false sharing
- Contadores locales

✅ **Nivel de CPU**:
- Precálculo de constantes
- Optimización de operaciones
- Flags agresivos de compilación

✅ **Nivel de Memoria**:
- Alineación de estructuras
- Localidad de datos
- Minimizar accesos compartidos

### Impacto Medido (Preliminar)

Con 4 threads en 10M iteraciones:

**Needles**:
- Serial: 0.268s
- OpenMP Basic: 0.068s → **Speedup: 3.94x**
- OpenMP Optimized: 0.093s (variabilidad)

**Dartboard**:
- Serial: 0.161s
- OpenMP Basic: 0.067s → **Speedup: 2.40x**
- OpenMP Optimized: 0.056s → **Speedup: 2.88x**

### Observaciones

1. Needles tiene mejor speedup (más trabajo por iteración)
2. Optimizaciones muestran resultados variables (necesita más runs)
3. Scheduling policy tiene impacto moderado
4. False sharing crítico para operaciones rápidas

---

**Próximos pasos**: Benchmarking exhaustivo y análisis de profiling para validar optimizaciones.
