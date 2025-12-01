# Modelo de Autómata Celular - Flujo de Tráfico - Análisis

## 1. Descripción del Modelo

### Estructura del Camino
- **N celdas** numeradas del 1 al N (camino circular/rotonda)
- Cada celda: `0` (vacía) o `1` (auto presente)
- **Condiciones de frontera periódicas**: celda 0 = celda N, celda N+1 = celda 1

### Reglas de Actualización
En cada paso de tiempo `t → t+1`, para cada celda `i`:
- **Si** la celda actual tiene un auto (`R_t(i) = 1`) **Y** la siguiente celda está vacía (`R_t(i+1) = 0`):
  - El auto **avanza** una celda
- **De lo contrario**:
  - El auto **permanece** en su lugar (si está bloqueado) o la celda permanece vacía

### Métrica de Velocidad
```
velocidad = (número de autos que se movieron) / (número total de autos)
```
- Rango: [0, 1]
- 0 = bloqueo completo (todos los autos bloqueados)
- 1 = flujo libre (todos los autos en movimiento)

## 2. Tablas de Verdad para las Reglas de Actualización

### Tabla 1: Celda actual VACÍA (R_t(i) = 0)

| R_t(i-1) | R_t(i) | R_t(i+1) | R_{t+1}(i) | Explicación                            |
|----------|--------|----------|------------|----------------------------------------|
| 0        | 0      | 0        | 0          | Vacía, ningún auto viene               |
| 0        | 0      | 1        | 0          | Vacía, auto adelante (no se mueve)     |
| 1        | 0      | 0        | 1          | **Auto desde la izquierda entra**      |
| 1        | 0      | 1        | 1          | **Auto desde la izquierda entra**      |

**Regla**: La celda se ocupa si el vecino izquierdo tiene un auto (ese auto avanza).

### Tabla 2: Celda actual OCUPADA (R_t(i) = 1)

| R_t(i-1) | R_t(i) | R_t(i+1) | R_{t+1}(i) | Explicación                            |
|----------|--------|----------|------------|----------------------------------------|
| 0        | 1      | 0        | 0          | **Auto avanza**                        |
| 0        | 1      | 1        | 1          | Auto bloqueado, permanece              |
| 1        | 1      | 0        | 0          | **Auto avanza**                        |
| 1        | 1      | 1        | 1          | Auto bloqueado, permanece              |

**Regla**: El auto avanza si la siguiente celda está vacía; permanece si está bloqueado.

### Regla Combinada Simplificada
```
R_{t+1}(i) = 1  si y solo si:
  - (R_t(i) = 1 Y R_t(i+1) = 1)  [auto bloqueado permanece]
  O
  - (R_t(i-1) = 1 Y R_t(i) = 0)  [auto desde la izquierda entra]
```

## 3. Pseudocódigo del Algoritmo Serial

### Método 1: Dos Fases (Claro y Simple)
```
Algoritmo: TrafficCA_Serial
Entrada: N (longitud del camino), T (pasos de tiempo), density (densidad inicial de autos)
Salida: velocities[T] (velocidad en cada paso de tiempo)

// Inicialización
old[N] ← initialize_random(density)  // o patrón específico
new[N] ← zeros(N)
total_cars ← count_ones(old)

para t = 1 hasta T:
    moved_count = 0
    
    // Fase 1: Determinar nuevo estado para cada celda
    para i = 0 hasta N-1:
        left  = old[(i - 1 + N) mod N]
        curr  = old[i]
        right = old[(i + 1) mod N]
        
        // Aplicar regla combinada
        si (curr == 1 Y right == 1):
            new[i] = 1  // bloqueado, permanece
        sino si (left == 1 Y curr == 0):
            new[i] = 1  // auto entra desde la izquierda
        sino:
            new[i] = 0  // vacía o auto se movió
    
    // Fase 2: Contar movimientos
    para i = 0 hasta N-1:
        si old[i] == 1 Y new[i] == 0:
            moved_count++
    
    // Actualizar y calcular velocidad
    swap(old, new)
    velocities[t] = moved_count / total_cars
    
retornar velocities
```

### Método 2: Lógica de Movimiento Directo (Alternativo)
```
para t = 1 hasta T:
    new[N] ← zeros(N)
    moved = 0
    
    para i = 0 hasta N-1:
        si old[i] == 1:  // celda tiene un auto
            next = (i + 1) mod N
            si old[next] == 0:  // puede moverse
                new[next] = 1
                moved++
            sino:  // bloqueado
                new[i] = 1
    
    swap(old, new)
    velocity = moved / total_cars
```

**Nota**: El Método 2 es más simple e implementa directamente la lógica de movimiento de autos. Ambos son equivalentes.

## 4. Condiciones de Frontera Periódicas

### Implementación
```c
// Para la celda i, los vecinos son:
int left  = (i - 1 + N) % N;  // maneja i=0 → envuelve a N-1
int right = (i + 1) % N;       // maneja i=N-1 → envuelve a 0
```

### Ejemplo (N=10)
```
Celdas: [0] [1] [2] [3] [4] [5] [6] [7] [8] [9]
         ↑__________________________________|  (periódica)
         |__________________________________↑
```
- Vecino izquierdo de celda 0: celda 9
- Vecino derecho de celda 9: celda 0

## 5. Complejidad Computacional

### Algoritmo Serial
- **Tiempo por iteración**: O(N)
  - Bucle a través de N celdas: O(N)
  - Cada celda: operaciones de tiempo constante O(1)
- **Tiempo total**: O(N × T)
- **Espacio**: O(N) para dos arreglos (old y new)

### Patrón de Acceso a Memoria
- **Secuencial**: Buena localidad de caché
- **Lectura**: Acceso a old[i-1], old[i], old[i+1]
- **Escritura**: Establecer new[i]
- **Dependencia de datos**: Ninguna entre diferentes i (embarazosamente paralelo por iteración)

## 6. Estrategia de Verificación

### Verificaciones de Correctitud
1. **Conservación de autos**: El total de autos debe permanecer constante
   ```
   assert(count(old) == total_cars) para todo t
   ```

2. **Checksum**: Suma de (posición × valor)
   ```
   checksum = Σ(i × old[i]) mod PRIMO_GRANDE
   ```
   - Estados diferentes deben tener checksums diferentes (con alta probabilidad)

3. **Límites de velocidad**: 
   ```
   0 ≤ velocity ≤ 1
   ```

4. **Determinismo**: Mismo estado inicial + misma semilla → mismos resultados

### Casos de Prueba
1. **Camino vacío** (density=0): velocidad indefinida (o 0)
2. **Camino lleno** (density=1): velocidad = 0 (bloqueo completo)
3. **Un solo auto** (density=1/N): velocidad = 1 (siempre se mueve)
4. **Alternante** (1,0,1,0,...): velocidad = 0 (todos bloqueados)
5. **Disperso** (density=0.3): velocidad ≈ 0.7–0.9

## 7. Comportamiento Esperado

### Transiciones de Fase
El flujo de tráfico exhibe comportamiento de transición de fase:
- **Baja densidad** (ρ < 0.3): Flujo libre, velocidad ≈ 1
- **Densidad media** (0.3 < ρ < 0.7): Flujo mixto, 0 < velocidad < 1
- **Alta densidad** (ρ > 0.7): Congestión, velocidad → 0
- **Densidad crítica** (ρ ≈ 0.5): Máximo rendimiento

### Patrones Emergentes
- **Atascos de tráfico**: Se pueden formar grupos de autos detenidos y propagarse hacia atrás
- **Soluciones periódicas**: Algunas configuraciones se repiten después de k pasos de tiempo
- **Comportamiento caótico**: Pequeños cambios en condiciones iniciales → grandes diferencias a largo plazo

## 8. Parámetros para Benchmarking

### Valores Recomendados
```
N (celdas):       100, 1000, 10000, 100000, 1000000
T (pasos):        100, 500, 1000, 5000
density:          0.3, 0.5, 0.7 (regímenes interesantes)
P (procesos):     1, 2, 4, 6 (para MPI)
repeticiones:     10 (para significancia estadística)
```

### Para Cluster t3.micro (1GB RAM)
```
Conservador:
  N = 100,000   (100KB por arreglo)
  T = 1,000
  
Moderado:
  N = 1,000,000 (1MB por arreglo)
  T = 1,000
  
Agresivo:
  N = 10,000,000 (10MB por arreglo) - ¡probar primero!
  T = 500
```

## 9. Aspectos Clave para Paralelización

### Dependencias de Datos
- Cada actualización de celda depende de **3 valores**: izquierda, actual, derecha
- **Sin dependencias entre iteraciones** (doble buffer)
- **Localidad espacial**: Solo se necesitan celdas adyacentes

### Estrategia de Paralelización
1. **Descomposición de dominio**: Dividir celdas entre procesos
2. **Celdas fantasma**: Cada proceso necesita celdas de frontera de vecinos
3. **Patrón de comunicación**: 
   - Enviar celda más a la derecha al vecino derecho
   - Enviar celda más a la izquierda al vecino izquierdo
   - Recibir de ambos vecinos
4. **Reducción global**: Sumar moved_count entre todos los procesos

### Relación Comunicación vs Computación
```
Computación: O(N/P) por proceso
Comunicación: O(1) mensajes × O(1) datos por iteración
Relación: O(N/P) → favorable para N grande
```

**Conclusión**: Se espera buena eficiencia paralela para N >> P, T >> 1.
