# Directorio de Resultados

Este directorio contiene los resultados de benchmarks ejecutados en diferentes entornos.

## Estructura

```
results/
├── local/                          # Resultados de tu MacBook Air M2
│   ├── benchmarks_*.csv           # CSVs de benchmarks locales
│   └── analysis/                   # Análisis de resultados locales
│       ├── statistics.csv
│       ├── speedup_efficiency.csv
│       ├── *.png                   # Gráficas
│       └── REPORTE_ANALISIS.txt
│
└── cluster/                        # Resultados del cluster AWS
    ├── benchmarks_*.csv           # CSVs de benchmarks del cluster
    └── analysis/                   # Análisis de resultados del cluster
        ├── statistics.csv
        ├── speedup_efficiency.csv
        ├── *.png
        └── REPORTE_ANALISIS.txt
```

## Uso

### Ejecutar Benchmarks Locales
```bash
# Desde el directorio raíz del proyecto
./scripts/run_benchmarks.sh local    # o simplemente sin argumento (default: local)
```

### Ejecutar Benchmarks en Cluster
```bash
# 1. Conectar al cluster
ssh -i ~/.ssh/hpc-reto1-key.pem ubuntu@18.224.187.40

# 2. Ir al directorio
cd /home/ubuntu/HPC/HPCReto3

# 3. Ejecutar con argumento "cluster"
./scripts/run_benchmarks.sh cluster

# O en background:
nohup ./scripts/run_benchmarks.sh cluster > benchmark.log 2>&1 &
```

### Descargar Resultados del Cluster
```bash
# Desde tu MacBook (directorio local del proyecto)
./scripts/download_cluster_results.sh
```

### Analizar Resultados

**Local:**
```bash
./scripts/analyze_latest.sh local    # Analiza último benchmark local
```

**Cluster:**
```bash
./scripts/analyze_latest.sh cluster  # Analiza último benchmark del cluster
```

### Comparar Local vs Cluster
```bash
python3 scripts/compare_environments.py
```

## Archivos de Benchmarks

Formato del nombre: `benchmarks_YYYYMMDD_HHMMSS.csv`

Ejemplo:
- `benchmarks_20251129_194333.csv` - Ejecutado el 29/Nov/2025 a las 19:43:33

Cada CSV contiene:
```csv
version,N,T,density,P,run,execution_time,mean_velocity,total_cars
serial,100000,100,0.5,1,1,0.012345,0.4567,50000
mpi_blocking,100000,100,0.5,2,1,0.008123,0.4568,50000
...
```

## Análisis Generado

En cada subdirectorio `analysis/` encontrarás:

### CSVs
- `statistics.csv` - Estadísticas por configuración (mean, std, CV%)
- `speedup_efficiency.csv` - Speedup y eficiencia calculados
- `version_summary.csv` - Resumen por versión MPI

### Gráficas
- `strong_scaling.png` - Speedup y eficiencia vs P
- `weak_scaling.png` - Tiempo vs P con N/P constante
- `blocking_vs_nonblocking.png` - Comparación MPI
- `density_analysis.png` - Efecto de densidad en velocidad
- `execution_time_heatmap.png` - Heatmap N vs P

### Gráficas de Presentación
En `analysis/plots_publication/`:
- `fig1_speedup.png` - Speedup vs Procesos (alta calidad)
- `fig2_efficiency.png` - Comparación de eficiencia
- `fig3_improvement.png` - Mejora Non-Blocking vs Blocking
- `fig4_summary.png` - Panel completo 2×2

### Reporte de Texto
- `REPORTE_ANALISIS.txt` - Resumen ejecutivo con:
  - Top 5 configuraciones
  - Comparación blocking vs non-blocking
  - Análisis de variabilidad
  - Strong scaling analysis

