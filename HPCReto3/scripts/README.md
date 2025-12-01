# HPCReto3 - Scripts de Análisis

Este directorio contiene scripts para análisis de benchmarks y generación de reportes.

## Scripts Disponibles

### 1. `run_benchmarks.sh`
Script principal de benchmarking que ejecuta todas las configuraciones de prueba.

**Uso:**
```bash
cd /path/to/HPCReto3
make all
./scripts/run_benchmarks.sh
```

**Características:**
- 10 repeticiones por configuración
- 7 fases de testing:
  - Baseline serial (3 tamaños: 100K, 500K, 1M)
  - Strong scaling (N=1.2M fijo, P=1,2,4,6)
  - Weak scaling (N/P=200K constante)
  - Densidades variables (0.3, 0.5, 0.7)
  - Muchos timesteps (T=1000)
- Output: `results/benchmarks_TIMESTAMP.csv`
- Total: ~280 tests (10 runs × 28 configs)

### 2. `analyze_results.py`
Analiza CSV de benchmarks y genera estadísticas + gráficas completas.

**Requisitos:**
```bash
pip3 install pandas numpy matplotlib seaborn
```

**Uso:**
```bash
python3 scripts/analyze_results.py results/benchmarks_20251129_143022.csv
```

**Output generado:**
- `results/analysis/statistics.csv` - Estadísticas por configuración
- `results/analysis/speedup_efficiency.csv` - Speedup y eficiencia
- `results/analysis/version_summary.csv` - Resumen por versión
- `results/analysis/strong_scaling.png` - Gráficas strong scaling
- `results/analysis/weak_scaling.png` - Gráficas weak scaling
- `results/analysis/blocking_vs_nonblocking.png` - Comparación MPI
- `results/analysis/density_analysis.png` - Efecto de densidad
- `results/analysis/execution_time_heatmap.png` - Heatmap de tiempos
- `results/analysis/REPORTE_ANALISIS.txt` - Reporte textual completo

### 3. `generate_plots.py`
Genera gráficas de alta calidad estilo publicación científica.

**Uso:**
```bash
python3 scripts/generate_plots.py results/analysis
```

**Output generado:**
- `results/analysis/plots_publication/fig1_speedup.png` - Speedup vs Procesos
- `results/analysis/plots_publication/fig2_efficiency.png` - Comparación Eficiencia
- `results/analysis/plots_publication/fig3_improvement.png` - Mejora Non-Blocking
- `results/analysis/plots_publication/fig4_summary.png` - Panel 2×2 completo

### 4. `test_serial.sh`
Suite de tests automatizados para versión serial.

**Uso:**
```bash
./scripts/test_serial.sh
```

**Tests incluidos:**
1. Road vacío (density=0)
2. Road lleno (density=1)
3. Sparse traffic (density=0.1)
4. Dense traffic (density=0.9)
5. Medium traffic (density=0.5)
6. Determinism check (misma semilla → mismo resultado)
7. Large road (N=1M)
8. Many timesteps (T=1000)
9. Conservation check
10. Phase transition (density=0.5)

### 5. `test_mpi_blocking.sh`
Suite de tests para versión MPI blocking.

**Uso:**
```bash
./scripts/test_mpi_blocking.sh
```

**Configuraciones testeadas:**
- P=1 (caso especial sin comunicación MPI)
- P=2, 4, 6 (diferentes descomposiciones)
- Verificación de conservación de coches
- Correctitud de velocidades

### 6. `deploy_to_cluster.sh`
Despliega código al cluster AWS y ejecuta tests.

**Requisitos:**
- Configurar IPs en el script
- SSH key configurado para ubuntu@IPs

**Uso:**
```bash
./scripts/deploy_to_cluster.sh
```

**Acciones:**
1. rsync de src/, Makefile, scripts/
2. Compilación remota con mpicc
3. Ejecución de test suite
4. Verificación de funcionamiento

## Workflow Completo

### Local (Desarrollo):
```bash
# 1. Compilar
cd HPCReto3
make all

# 2. Ejecutar benchmarks
./scripts/run_benchmarks.sh
# Output: results/benchmarks_TIMESTAMP.csv

# 3. Análisis estadístico
python3 scripts/analyze_results.py results/benchmarks_TIMESTAMP.csv
# Output: results/analysis/

# 4. Gráficas de presentación
python3 scripts/generate_plots.py results/analysis
# Output: results/analysis/plots_publication/
```

### Cluster AWS:
```bash
# 1. Desplegar
./scripts/deploy_to_cluster.sh

# 2. Conectar al nodo master
ssh ubuntu@172.31.28.209

# 3. Ejecutar benchmarks
cd HPC/HPCReto3
./scripts/run_benchmarks.sh

# 4. Descargar resultados
scp ubuntu@172.31.28.209:/home/ubuntu/HPC/HPCReto3/results/benchmarks_*.csv ./results/

# 5. Analizar localmente
python3 scripts/analyze_results.py results/benchmarks_cluster.csv
```

## Formato CSV de Benchmarks

```csv
version,N,T,density,P,run,execution_time,mean_velocity,total_cars
serial,100000,100,0.5,1,1,0.012345,0.4567,50000
mpi_blocking,100000,100,0.5,2,1,0.008123,0.4568,50000
mpi_nonblocking,100000,100,0.5,2,1,0.006789,0.4569,50000
...
```

## Métricas Calculadas

### Speedup
```
S(P) = T_serial / T_parallel(P)
```

### Eficiencia
```
E(P) = S(P) / P × 100%
```

### Mejora de Non-Blocking
```
Improvement = (T_blocking - T_nonblocking) / T_blocking × 100%
```

### Coeficiente de Variación
```
CV = (std / mean) × 100%
```

## Dependencias Python

```bash
pip3 install pandas numpy matplotlib seaborn
```

**Versiones recomendadas:**
- pandas >= 1.3.0
- numpy >= 1.21.0
- matplotlib >= 3.4.0
- seaborn >= 0.11.0

## Notas

- Todos los scripts deben ejecutarse desde el directorio raíz de HPCReto3
- Los benchmarks pueden tardar varios minutos (10 runs × muchas configs)
- El análisis Python requiere datos completos de benchmarks
- Las gráficas se generan en formato PNG a 300 DPI
- Para cluster AWS, usar N divisibles por 2, 4, 6 (compatibilidad con P)

## Autor

Isaac Pachon - 2025
Universidad Tecnológica de Panamá
HPCReto3: Autómata Celular con MPI
