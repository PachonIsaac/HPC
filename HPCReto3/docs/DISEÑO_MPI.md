# Diseño de Paralelización MPI para Autómata Celular

## 1. Estrategia de Descomposición de Dominio

### Particionamiento 1D
Cada proceso posee un bloque contiguo de celdas:

```
Total de celdas: N (debe ser divisible por P para simplicidad)
Tamaño local por proceso: local_N = N / P

Proceso 0:   celdas [0 ... local_N-1]
Proceso 1:   celdas [local_N ... 2*local_N-1]
...
Proceso P-1: celdas [(P-1)*local_N ... N-1]
```

### Celdas Fantasma (Regiones Halo)
Cada proceso necesita valores de frontera de vecinos:

```
Almacenamiento local del proceso i:
[fantasma_izq] [celdas locales: 0...local_N-1] [fantasma_der]

fantasma_izq  = celda más a la derecha del proceso (i-1+P) % P
fantasma_der = celda más a la izquierda del proceso (i+1) % P
```

**Ejemplo con N=12, P=3:**
```
Global:  [0][1][2][3] [4][5][6][7] [8][9][10][11]
         └─ P0 ───┘   └─ P1 ───┘   └─ P2 ────┘

P0 local: [11][0][1][2][3][4]     // fantasma_izq=11, fantasma_der=4
P1 local: [3][4][5][6][7][8]      // fantasma_izq=3, fantasma_der=8
P2 local: [7][8][9][10][11][0]    // fantasma_izq=7, fantasma_der=0
```

## 2. Patrón de Comunicación (Bloqueante)

### Algoritmo por Paso de Tiempo
```
1. Intercambiar celdas fantasma (bloqueante MPI_Sendrecv):
   - Enviar celda local más a la derecha al vecino derecho
   - Recibir fantasma izquierdo del vecino izquierdo
   - Enviar celda local más a la izquierda al vecino izquierdo
   - Recibir fantasma derecho del vecino derecho

2. Actualizar celdas locales usando reglas CA:
   - Usar celdas fantasma para actualizaciones de frontera
   - Todas las celdas locales actualizadas independientemente

3. Contar movimientos locales:
   - Comparar estado antiguo vs nuevo

4. Reducción global (MPI_Reduce):
   - Sumar moved_count entre todos los procesos
   - Calcular velocidad global
```

### Detalles de Comunicación MPI

#### Intercambio de Fantasmas (Bloqueante)
```c
// Intercambio con vecino derecho
MPI_Sendrecv(
    &local_old[local_N-1], 1, MPI_INT, right_rank, TAG_RIGHT,
    &ghost_left, 1, MPI_INT, left_rank, TAG_LEFT,
    MPI_COMM_WORLD, &status);

// Intercambio con vecino izquierdo
MPI_Sendrecv(
    &local_old[0], 1, MPI_INT, left_rank, TAG_LEFT,
    &ghost_right, 1, MPI_INT, right_rank, TAG_RIGHT,
    MPI_COMM_WORLD, &status);
```

**Ventajas:**
- Simple, libre de deadlocks (MPI lo maneja)
- Una llamada de función por dirección de intercambio

**Desventajas:**
- Comunicación secuencial (no se puede solapar con computación)

#### Reducción Global
```c
int local_moved, global_moved;
// ... contar local_moved ...

MPI_Reduce(&local_moved, &global_moved, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
```

Solo el rango 0 obtiene el resultado, hace broadcast de la velocidad si es necesario.

## 3. Balanceo de Carga

### Descomposición Estática
- Cada proceso: mismo número de celdas (N/P)
- No se necesita rebalanceo dinámico
- **Suposición:** N es divisible por P

### Distribución del Trabajo
```
Computación por proceso: O(local_N) = O(N/P)
Comunicación por proceso: O(1) por paso de tiempo
   - 2 envíos + 2 recepciones de 1 celda cada uno
   - 1 MPI_Reduce (complejidad O(log P))
```

### Condición de Speedup Ideal
```
Speedup ≈ P  cuando:
   T_comp >> T_comm
   (N/P) >> latencia + (2 celdas / ancho_banda)

Para N grande (> 100,000), la latencia es despreciable.
```

## 4. Comunicación No Bloqueante (Futuro)

### Solapar Computación con Comunicación
```
1. MPI_Isend/MPI_Irecv: Publicar envíos/recepciones no bloqueantes
2. Actualizar celdas interiores (no necesitan celdas fantasma aún)
3. MPI_Waitall: Esperar que complete la comunicación
4. Actualizar celdas de frontera (ahora tienen datos fantasma)
5. MPI_Reduce: Suma global
```

**Ventajas:**
- Ocultar latencia de comunicación
- Mejor utilización de CPU durante transferencias de red

**Implementación:**
```c
MPI_Request requests[4];
MPI_Isend(&local_old[local_N-1], 1, MPI_INT, right_rank, TAG, ..., &requests[0]);
MPI_Irecv(&ghost_left, 1, MPI_INT, left_rank, TAG, ..., &requests[1]);
MPI_Isend(&local_old[0], 1, MPI_INT, left_rank, TAG, ..., &requests[2]);
MPI_Irecv(&ghost_right, 1, MPI_INT, right_rank, TAG, ..., &requests[3]);

// Actualizar interior: local_old[1 ... local_N-2]
update_interior(local_old, local_new, 1, local_N-2);

MPI_Waitall(4, requests, statuses);

// Actualizar fronteras: local_old[0] y local_old[local_N-1]
update_boundaries(local_old, local_new, ghost_left, ghost_right);
```

## 5. Estrategia de Inicialización

### Opción A: Root Inicializa y Distribuye
```
Rango 0:
  - Inicializar arreglo completo de N celdas
  - MPI_Scatter para distribuir bloques

Todos los rangos:
  - Recibir local_N celdas
```

**Pros:** Simple, reproducible (misma semilla → misma distribución)  
**Contras:** Cuello de botella de memoria en rango 0 para N grande

### Opción B: Cada Proceso Inicializa Localmente
```
Cada rango:
  - Semilla: semilla_base + rango (para reproducibilidad)
  - Inicializar local_N celdas independientemente
```

**Pros:** Escalable, sin cuello de botella de memoria  
**Contras:** Necesita coordinar semillas para reproducibilidad

**Elección para Reto3:** Opción B (escalable)

## 6. Estrategia de Verificación

### Verificaciones de Correctitud
1. **Conservación entre procesos:**
   ```c
   int local_cars = count_local(local_old, local_N);
   int global_cars;
   MPI_Reduce(&local_cars, &global_cars, 1, MPI_INT, MPI_SUM, 0, ...);
   // Rango 0: verificar global_cars == initial_global_cars
   ```

2. **Consistencia de frontera:**
   - Después del intercambio fantasma, verificar que celdas fantasma coincidan con frontera del vecino

3. **Determinismo:**
   - Mismos N, T, density, seed, P → misma velocidad global

4. **Equivalencia serial (N pequeño):**
   - Comparar salida MPI con serial para mismos parámetros

### Casos de Prueba
```bash
# Un solo proceso (debe coincidir exactamente con serial)
mpirun -np 1 ./ca_mpi_blocking 1000 100 0.5 42

# Comparar con serial
./ca_serial 1000 100 0.5 42

# Multi-proceso (prueba de conservación)
mpirun -np 4 ./ca_mpi_blocking 1000 100 0.5 42
```

## 7. Métricas de Rendimiento

### Qué Medir
```
1. Tiempo total de ejecución (MPI_Wtime)
2. Tiempo de comunicación (tiempo en MPI_Sendrecv + MPI_Reduce)
3. Tiempo de computación (bucle timestep - comunicación)
4. Speedup: S(P) = T_serial / T_parallel(P)
5. Eficiencia: E(P) = S(P) / P
6. Escalabilidad: débil/fuerte
```

### Parámetros de Benchmark
```
Escalado fuerte (N fijo, variar P):
  N = 1,000,000
  P = 1, 2, 4, 6
  
Escalado débil (N/P fijo, variar P):
  N/P = 200,000
  P = 1, 2, 4, 6
  N = 200K, 400K, 800K, 1.2M
```

### Rendimiento Esperado
```
Para N = 1,000,000, T = 1000, density = 0.5:

Serial:        ~2.5 segundos
MPI P=2:       ~1.3 segundos (speedup ≈ 1.9)
MPI P=4:       ~0.7 segundos (speedup ≈ 3.5)
MPI P=6:       ~0.5 segundos (speedup ≈ 5.0)

El overhead de comunicación aumenta con P, por lo que la eficiencia baja:
E(2) ≈ 95%
E(4) ≈ 88%
E(6) ≈ 83%
```

## 8. Estructura del Código

### Organización de Archivos
```
ca_mpi_blocking.c:
  - main()
  - initialize_local_road()
  - exchange_ghosts_blocking()
  - update_local_timestep()
  - global_reduction()
  - verify_conservation()

ca_mpi_nonblocking.c:
  - Misma estructura
  - exchange_ghosts_nonblocking()
  - update_interior()
  - update_boundaries()
```

### Interfaz de Línea de Comandos
```
mpirun -np <P> ./ca_mpi_blocking <N> <T> <density> [seed]

Requisitos:
  - N debe ser divisible por P
  - Validar al inicio, abortar si no
```

### Formato de Salida
```
Solo el rango 0 imprime:
  - Parámetros (N, T, density, P)
  - Conteo inicial global de autos
  - Velocidad por paso de tiempo (o resumen)
  - Verificación final (conservación, checksum)
  - Métricas de rendimiento (tiempo, speedup)
```

## 9. Condiciones de Frontera Periódicas en MPI

### Cálculo de Rangos
```c
int rank, size;
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);

// Topología circular
int left_rank = (rank - 1 + size) % size;  // envuelve: 0 → P-1
int right_rank = (rank + 1) % size;        // envuelve: P-1 → 0
```

### Ejemplo con P=4
```
Proceso 0: left=3, right=1
Proceso 1: left=0, right=2
Proceso 2: left=1, right=3
Proceso 3: left=2, right=0
```

Esto implementa naturalmente las condiciones de frontera periódicas en todo el camino.

## 10. Consejos de Depuración

### Errores Comunes
1. **Deadlock en comunicación bloqueante:**
   - Usar `MPI_Sendrecv` (no `MPI_Send`/`MPI_Recv` separados)

2. **Celdas fantasma no actualizadas:**
   - Verificar cálculos de rango para vecinos
   - Verificar buffers de envío/recepción

3. **Violación de conservación:**
   - Revisar actualizaciones de celdas de frontera
   - Asegurar que no se pierdan celdas en fronteras de dominio

4. **N no divisible por P:**
   - Agregar validación: `if (N % P != 0) MPI_Abort(...)`

### Flags de Depuración
```bash
# Ejecutar con N pequeño para visualizar
mpirun -np 2 ./ca_mpi_blocking 10 5 0.5 42

# Usar depuración MPI
export OMPI_MCA_mpi_show_mca_params=1
```

## 11. Checklist de Implementación

### Versión MPI Bloqueante
- [ ] Inicializar MPI (MPI_Init, MPI_Comm_rank, MPI_Comm_size)
- [ ] Validar N % P == 0
- [ ] Calcular local_N, left_rank, right_rank
- [ ] Inicializar camino local (semilla = semilla_base + rango)
- [ ] Asignar local_old, local_new, celdas fantasma
- [ ] Bucle principal:
  - [ ] Intercambiar fantasmas (MPI_Sendrecv × 2)
  - [ ] Actualizar celdas locales con reglas CA
  - [ ] Contar movimientos locales
  - [ ] MPI_Reduce para global moved_count
  - [ ] Rango 0: calcular e imprimir velocidad
- [ ] Verificación:
  - [ ] MPI_Reduce para verificación de conservación
  - [ ] Imprimir estadísticas finales (rango 0)
- [ ] Finalizar MPI
- [ ] Agregar medición de tiempo (MPI_Wtime)

### Pruebas
- [ ] Un solo proceso coincide con serial
- [ ] Multi-proceso conservación
- [ ] Determinismo (misma semilla → mismo resultado)
- [ ] Rendimiento escala con P

---

**Listo para implementar:** El siguiente paso es codificar `ca_mpi_blocking.c` siguiendo este diseño.
