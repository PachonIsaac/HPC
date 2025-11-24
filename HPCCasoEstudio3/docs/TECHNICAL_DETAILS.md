# Technical Implementation Details - MPI Matrix Multiplication

## Algorithm Analysis

### Row-wise Matrix Distribution

Para multiplicación de matrices C = A × B donde todas son n×n:

**Distribución**:
- Matriz A: dividida horizontalmente en strips de filas
- Matriz B: completa enviada a todos los procesos
- Matriz C: cada proceso calcula su strip de filas

**Complejidad**:
- Sequential: O(n³) 
- Parallel con p procesos: O(n³/p) + comunicación

**Comunicación**:
- Scatter de A: O(n²/p) datos por proceso
- Broadcast de B: O(n²) datos a todos
- Gather de C: O(n²/p) datos por proceso
- Total: O(n²)

### Implementation Strategies

#### 1. Sequential Baseline (`matrix_mpi_sequential.c`)

**Propósito**: Medir overhead puro de MPI framework sin paralelización.

**Características**:
- Solo rank 0 ejecuta cómputo
- Otros ranks quedan idle
- Útil para calcular overhead: `overhead = T_mpi_seq - T_serial`

**Código clave**:
```c
if (rank == 0) {
    matrix_multiply(A, B, C, matrix_size);  // Solo maestro trabaja
}
```

#### 2. Row-wise Distribution (`matrix_mpi_rowwise.c`)

**Estrategia**: Distribución estática de filas equitativamente.

**Ventajas**:
- Simple de implementar
- Load balancing natural (misma carga por proceso)
- Usa collective communications (MPI_Scatter/Gather)

**Desventajas**:
- Broadcast de B completo (n² datos) a cada proceso
- Sincronización global en Scatter/Gather
- No overlap de comunicación/cómputo

**Código clave**:
```c
// 1. Distribución
MPI_Scatter(A, local_rows*n, ..., A_local, ...);
MPI_Bcast(B, n*n, ...);

// 2. Cómputo local
matrix_multiply_rows(A_local, B, C_local, local_rows, n);

// 3. Recolección
MPI_Gather(C_local, local_rows*n, ..., C, ...);
```

**Complejidad comunicación**:
- Scatter: O(n²/p) por proceso
- Bcast: O(log p × n²) (árbol binomial)
- Gather: O(n²/p) por proceso
- Total: O(log p × n²)

#### 3. Broadcast Optimized (`matrix_mpi_broadcast.c`)

**Optimización**: Reduce número de operaciones colectivas.

**Mejoras vs rowwise**:
- Mismo algoritmo pero con timing más detallado
- Mide separadamente tiempo de comunicación vs cómputo
- Calcula métricas de load balance
- Usa `MPI_Reduce` para recopilar estadísticas de todos los procesos

**Métricas adicionales**:
```c
// Load balance: ¿todos los procesos tardan igual?
load_balance = min_compute_time / max_compute_time * 100%

// Communication overhead
comm_overhead = max_comm_time / total_time * 100%
```

**Por qué es mejor**:
- Visibilidad: identifica cuellos de botella
- Diagnóstico: detecta desbalance de carga
- Optimización guiada: métricas para mejorar

#### 4. Non-blocking Communication (`matrix_mpi_nonblocking.c`)

**Estrategia**: Overlap comunicación con cómputo usando MPI no bloqueante.

**Ventajas**:
- `MPI_Isend/MPI_Irecv` permiten continuar ejecutando mientras datos se transfieren
- Potencial para reducir tiempo efectivo de comunicación
- Útil cuando latencia de red es significativa

**Desventajas**:
- Más complejo de implementar
- Requiere manejo manual de requests y status
- Beneficio depende de ratio computation/communication

**Código clave**:
```c
// Master: envío no bloqueante
MPI_Request requests[num_procs];
for (int i = 1; i < num_procs; i++) {
    MPI_Isend(B, n*n, MPI_DOUBLE, i, TAG_B, ..., &requests[i-1]);
    MPI_Isend(&A[i*local_rows*n], local_rows*n, ..., &requests[i-1]);
}

// Mientras se envían datos, master puede comenzar su cómputo
matrix_multiply_rows(A_local, B, C_local, local_rows, n);

// Workers: recepción no bloqueante
MPI_Request req_B, req_A;
MPI_Irecv(B, n*n, MPI_DOUBLE, 0, TAG_B, ..., &req_B);
MPI_Irecv(A_local, local_rows*n, ..., 0, TAG_A, ..., &req_A);

// Wait solo cuando necesitamos los datos
MPI_Wait(&req_B, ...);
MPI_Wait(&req_A, ...);

// Ahora computar
matrix_multiply_rows(...);
```

**Overlap potencial**:
```
Sin overlap:      |--comm--|--compute--|
Con overlap:      |--comm--|
                     |--compute--|
Total time saved: comunicación parcialmente escondida por cómputo
```

## Performance Considerations

### Memory Requirements

Para matriz n×n con doubles (8 bytes):

**Por proceso**:
- `A_local`: (n/p) × n × 8 bytes
- `B`: n × n × 8 bytes  ← Completa!
- `C_local`: (n/p) × n × 8 bytes
- **Total**: n²(8 + 16/p) bytes

**Para n=2048, p=6**:
- B: 2048² × 8 = 33.5 MB
- A_local: (2048/6) × 2048 × 8 ≈ 5.6 MB
- C_local: 5.6 MB
- **Total per process**: ~45 MB

✅ **Seguro** para t3.micro (1GB RAM) con margen amplio.

**Límites**:
- n=4096: ~180 MB por proceso → OK
- n=8192: ~700 MB por proceso → Riesgoso (poco margen)
- n=16384: ~2.8 GB por proceso → **OOM** garantizado

### Network Considerations

**AWS t3.micro networking**:
- Bandwidth: hasta 5 Gbps
- Latency entre nodos: ~0.5-1ms (same AZ)
- Para n=1024: broadcast 8MB → ~13ms @ 5Gbps + latency

**Communication vs Computation ratio**:
```
T_comm / T_comp = (α × n² × log p) / (β × n³ / p)
                = (α × log p × p) / (β × n)
```

Donde:
- α = latency + 1/bandwidth
- β = tiempo por operación FLOP

**Implicación**: Matrices pequeñas → comunicación domina
                Matrices grandes → cómputo domina

**Break-even point** (estimado para cluster AWS):
- n < 512: comunicación > 50% del tiempo (poor speedup)
- n = 1024: comunicación ~30% del tiempo (decent speedup)
- n = 2048: comunicación ~15% del tiempo (good speedup)
- n ≥ 4096: comunicación < 10% del tiempo (excellent speedup)

### Expected Speedup

**Modelo teórico (Amdahl's Law)**:
```
S(p) = 1 / (s + (1-s)/p)
```
Donde s = fracción secuencial

Para multiplicación de matrices con comunicación:
```
T_parallel = T_comp/p + T_comm + T_overhead
S(p) = T_serial / T_parallel
```

**Para nuestro cluster (6 procesos)**:
- **Speedup ideal**: 6x
- **Speedup realista (n=1024)**: 3-4x (50-67% eficiencia)
- **Speedup realista (n=2048)**: 4-5x (67-83% eficiencia)

**Factores limitantes**:
1. Broadcast de matriz B (no escalable)
2. Gather de resultados (sincronización)
3. Load imbalance (si n no divisible por p)
4. Overhead MPI framework

### Optimization Opportunities

#### 1. Block Distribution (not implemented)
En vez de distribuir filas completas, distribuir bloques 2D:
- Reduce broadcast: cada proceso solo necesita subset de B
- Mejor cache locality
- Complejidad: mayor, requiere comunicaciones punto-a-punto

#### 2. Cannon's Algorithm (not implemented)
Algoritmo especializado para matrices distribuidas:
- Ambas A y B distribuidas
- Comunicación rolling (cada proceso rota sus datos)
- Optimal: O(n³/p) cómputo, O(n²/√p) comunicación
- Complejidad: requiere topología mesh

#### 3. Strassen's Algorithm (not implemented)
Reduce complejidad de O(n³) a O(n^2.807):
- Menos operaciones aritméticas
- Más comunicación (submatrices recursivas)
- Trade-off: bueno para matrices grandes

#### 4. Cache Optimization
Implementado implícitamente en cómputo:
```c
// Loop order: i-j-k (row-major, cache-friendly para C)
for (int i = 0; i < local_rows; i++) {
    for (int j = 0; j < size; j++) {
        C[i*size + j] = 0.0;
        for (int k = 0; k < size; k++) {
            C[i*size + j] += A[i*size + k] * B[k*size + j];
        }
    }
}
```

**Mejoras posibles**:
- Blocking/tiling: procesar en tiles que caben en L1 cache
- Transpose B: mejorar acceso secuencial
- Vectorización: usar SIMD instrucciones

## Benchmarking Methodology

### Test Configuration

**Independent variables**:
- Matrix size: n ∈ {512, 1024, 2048}
- Process count: p ∈ {2, 4, 6}
- Implementation: {sequential, rowwise, broadcast, nonblocking}

**Dependent variables**:
- Execution time (wall clock)
- Speedup: S(p) = T(1) / T(p)
- Efficiency: E(p) = S(p) / p
- Communication time (where measurable)
- Computation time (where measurable)

**Fixed parameters**:
- Matrix initialization: deterministic (seed=12345, 54321)
- Number of runs: 1 per configuration (reproducible en cluster idle)
- Timeout: 300 seconds por test

### Metrics Calculation

**Speedup**:
```python
baseline_time = time(sequential, n, p=2)  # Menor número de procesos
speedup = baseline_time / time(implementation, n, p)
```

**Efficiency**:
```python
efficiency = (speedup / num_processes) * 100  # Porcentaje
```

**Ideal efficiency**: 100% (speedup lineal)
**Good efficiency**: >80%
**Acceptable**: 60-80%
**Poor**: <60%

### Statistical Considerations

**Variability sources**:
1. Network congestion (otros usuarios en AWS)
2. CPU frequency scaling
3. OS scheduler decisions
4. Cache state inicial

**Mitigation**:
- Ejecutar en horas de baja carga
- Usar cluster dedicado (si posible)
- Warming run antes de benchmark real (no implementado)
- Múltiples runs y tomar mediana (no implementado por tiempo)

## Validation

### Correctness Testing

**Método**: Comparar resultados con implementación secuencial conocida.

**Sample points verificados**:
- C[0][0]: primer elemento
- C[n-1][n-1]: último elemento
- Suma de toda la matriz (checksum)

**Nota**: En producción, usar tests más exhaustivos:
```c
// No implementado aquí, pero recomendado:
bool verify_result(double *C_parallel, double *C_serial, int n) {
    double epsilon = 1e-9;
    for (int i = 0; i < n*n; i++) {
        if (fabs(C_parallel[i] - C_serial[i]) > epsilon) {
            return false;
        }
    }
    return true;
}
```

## Future Work

1. **Implementar SUMMA algorithm**: Scalable Universal Matrix Multiply Algorithm
2. **Non-blocking collectives**: MPI_Ibcast, MPI_Igather (MPI-3 standard)
3. **Process topology**: MPI_Cart_create para mejor mapping
4. **Asynchronous progress**: MPI_THREAD_MULTIPLE para threads + MPI
5. **GPU acceleration**: MPI + CUDA para hybrid parallelism
6. **I/O optimization**: MPI-IO para matrices grandes desde disco
7. **Dynamic load balancing**: Ajustar distribución según heterogeneidad

## References

### Academic Papers
- Fox et al. (1988) - "Solving Problems on Concurrent Processors"
- Kumar et al. (1994) - "Introduction to Parallel Computing"
- Cannon (1969) - "A Cellular Computer to Implement the Kalman Filter Algorithm"

### Documentation
- MPI Standard 4.0: https://www.mpi-forum.org/docs/
- OpenMPI Best Practices: https://www.open-mpi.org/papers/
- AWS HPC Guide: https://aws.amazon.com/hpc/

### Benchmarking
- LINPACK: https://www.netlib.org/benchmark/hpl/
- Intel MPI Benchmarks: https://software.intel.com/content/www/us/en/develop/articles/intel-mpi-benchmarks.html
