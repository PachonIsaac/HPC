# HPCCasoEstudio3 - Multiplicación de Matrices con MPI

## Descripción
Implementación de multiplicación de matrices usando MPI (Message Passing Interface) en un cluster AWS de 3 nodos (6 procesos).

## Cluster Configuration
- **Nodos**: 3 x AWS EC2 t3.micro instances
- **Recursos**: 1 vCPU, 1GB RAM por nodo
- **Red privada**: 172.31.x.x
- **Head node IP pública**: 18.224.187.40
- **MPI**: OpenMPI/MPICH
- **Procesos totales**: 6 (2 por nodo)

## Implementaciones

### 1. `matrix_mpi_sequential.c`
Baseline secuencial usando MPI pero ejecutando solo en rank 0.
- **Propósito**: Medir overhead de MPI framework
- **Estrategia**: Todo el trabajo en proceso maestro

### 2. `matrix_mpi_rowwise.c`
Distribución por filas usando MPI_Scatter/Gather.
- **Estrategia**: Master distribuye filas de matriz A
- **Comunicación**: `MPI_Scatter` para A, `MPI_Bcast` para B, `MPI_Gather` para resultados
- **Load balancing**: Filas equitativamente distribuidas

### 3. `matrix_mpi_broadcast.c`
Optimización de comunicación con broadcasting.
- **Estrategia**: Broadcast de B a todos, scatter de A
- **Optimización**: Reduce comunicación punto-a-punto
- **Métricas**: Calcula load balance y overhead de comunicación

### 4. `matrix_mpi_nonblocking.c`
Comunicación no bloqueante para overlap computation/communication.
- **Estrategia**: `MPI_Isend/MPI_Irecv` para comunicación asíncrona
- **Optimización**: Overlap entre comunicación y cómputo
- **Pipeline**: Recibe siguiente chunk mientras procesa actual

## Estructura del Proyecto
```
HPCCasoEstudio3/
├── src/                          # Código fuente
│   ├── matrix_mpi_sequential.c
│   ├── matrix_mpi_rowwise.c
│   ├── matrix_mpi_broadcast.c
│   └── matrix_mpi_nonblocking.c
├── bin/                          # Binarios compilados
├── scripts/
│   ├── deploy.sh                 # Despliega binarios al cluster
│   ├── run_benchmarks.sh         # Ejecuta benchmarks
│   └── analyze_results.py        # Análisis y visualizaciones
├── results/                      # Resultados de benchmarks
│   ├── benchmarks.csv
│   ├── processed_results.csv
│   ├── summary_report.txt
│   └── *.png                     # Gráficas
├── docs/                         # Documentación
├── Makefile                      # Build system
└── hostfile                      # Configuración MPI cluster

## Compilación

### Local (desarrollo)
```bash
make clean
make
```

### En cluster AWS
```bash
# En head node
make clean
make

# Desplegar a workers
chmod +x scripts/deploy.sh
./scripts/deploy.sh
```

## Ejecución

### Prueba individual
```bash
# Sequential baseline
mpirun -np 2 ./bin/matrix_mpi_sequential 512

# Row-wise distribution
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_rowwise 1024

# Broadcast optimized
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_broadcast 2048

# Non-blocking
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_nonblocking 1024
```

### Benchmarks completos
```bash
chmod +x scripts/run_benchmarks.sh
./scripts/run_benchmarks.sh
```

Prueba con:
- Tamaños de matriz: 512x512, 1024x1024, 2048x2048
- Número de procesos: 2, 4, 6
- Todas las implementaciones

## Análisis de Resultados

```bash
# Requiere: pandas, numpy, matplotlib, seaborn
python3 scripts/analyze_results.py
```

Genera:
- `speedup_comparison.png` - Speedup vs número de procesos
- `efficiency_comparison.png` - Eficiencia paralela
- `execution_times.png` - Comparación de tiempos
- `speedup_heatmap.png` - Heatmap de speedup
- `summary_report.txt` - Reporte estadístico

## Métricas Calculadas

### Speedup
```
S(p) = T₁ / Tₚ
```
Donde T₁ = tiempo con mínimo número de procesos, Tₚ = tiempo con p procesos

### Eficiencia
```
E(p) = S(p) / p × 100%
```

### Overhead de Comunicación
```
Comm% = t_comm / t_total × 100%
```

## Consideraciones AWS t3.micro

⚠️ **Limitaciones de RAM**: 1GB por nodo
- Matrices grandes (>2048) pueden causar OOM
- Usar matrices 512-2048 para tests confiables
- Monitorear uso de memoria: `free -h`

⚠️ **Red privada**: Sin NFS compartido
- Deployment manual con `scp`
- Binarios deben copiarse a cada worker

⚠️ **CPU limitada**: 1 vCPU por nodo
- No usar >2 procesos por nodo para evitar contention
- Configurar `slots=2` en hostfile

## Troubleshooting

### Error: "Matrix size must be divisible by number of processes"
```bash
# Usar tamaños divisibles por número de procesos
# Para 6 procesos: 512, 1024, 2048, 3072
mpirun -np 6 ./bin/matrix_mpi_rowwise 1024  # ✓
mpirun -np 6 ./bin/matrix_mpi_rowwise 1000  # ✗
```

### Error: MPI no encuentra workers
```bash
# Verificar hostfile
cat hostfile

# Probar conectividad
ssh worker1 hostname
ssh worker2 hostname

# Test MPI
mpirun --hostfile hostfile -np 6 hostname
```

### Out of Memory
```bash
# Reducir tamaño de matriz
mpirun -np 6 ./bin/matrix_mpi_rowwise 512  # Más seguro

# Monitorear memoria durante ejecución
ssh worker1 'watch -n1 free -h'
```

## Resultados Esperados

Para cluster de 6 procesos (3 nodos):
- **Speedup ideal**: 6x
- **Speedup realista**: 3-5x (overhead de comunicación)
- **Mejor implementación**: broadcast o non-blocking (menor comm overhead)
- **Peor caso**: sequential (overhead MPI sin paralelismo)

## Referencias
- MPI Tutorial: https://mpitutorial.com/
- OpenMPI Documentation: https://www.open-mpi.org/doc/
- AWS EC2 Networking: https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/

## Autor
Isaac Pachon
Universidad Tecnológica de Pereira
High Performance Computing - Caso de Estudio 3
