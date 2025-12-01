#!/bin/bash

# Script de Benchmarking para HPCReto3
# Ejecuta 10 repeticiones de cada configuración y genera CSV
# Autor: Isaac Pachon
# Fecha: 2025-11-29

set -e

# Configuración
NUM_RUNS=10
ENVIRONMENT="${1:-local}"  # Argumento: local o cluster (default: local)
OUTPUT_DIR="results/${ENVIRONMENT}"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
OUTPUT_CSV="${OUTPUT_DIR}/benchmarks_${TIMESTAMP}.csv"
HOSTFILE="hostfile"

# Crear directorio de resultados
mkdir -p "$OUTPUT_DIR"

# Verificar hostfile para MPI
if [ ! -f "$HOSTFILE" ]; then
    echo "⚠️  WARNING: hostfile no encontrado, creando uno por defecto..."
    echo "localhost slots=6" > "$HOSTFILE"
    echo "✓ hostfile creado: $HOSTFILE"
fi

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "========================================"
echo "HPCReto3 - Benchmark Suite"
echo "========================================"
echo ""
echo "Configuración:"
echo "  Entorno:          $ENVIRONMENT"
echo "  Repeticiones:     $NUM_RUNS"
echo "  Output:           $OUTPUT_CSV"
echo "  Timestamp:        $TIMESTAMP"
echo "========================================"
echo ""

# Verificar que los binarios existan
BINARIES=("bin/ca_serial" "bin/ca_mpi_blocking" "bin/ca_mpi_nonblocking")
for BIN in "${BINARIES[@]}"; do
    if [ ! -f "$BIN" ]; then
        echo -e "${RED}Error: $BIN no encontrado. Ejecuta 'make' primero.${NC}"
        exit 1
    fi
done

# Crear header del CSV
echo "version,N,T,density,P,run,execution_time,mean_velocity,total_cars" > "$OUTPUT_CSV"
echo -e "${GREEN}✓ CSV creado: $OUTPUT_CSV${NC}"
echo ""

# Función para ejecutar benchmark
run_benchmark() {
    local version=$1
    local binary=$2
    local N=$3
    local T=$4
    local density=$5
    local P=$6
    local seed_base=$7
    
    echo -e "${YELLOW}Testing: ${version} | N=$N, T=$T, P=$P${NC}"
    
    for run in $(seq 1 $NUM_RUNS); do
        # Semilla diferente por run para variabilidad
        local seed=$((seed_base + run))
        
        # Ejecutar y extraer métricas
        if [ "$version" == "serial" ]; then
            OUTPUT=$(./$binary $N $T $density $seed 2>&1)
        else
            OUTPUT=$(mpirun --hostfile "$HOSTFILE" -np $P $binary $N $T $density $seed 2>&1)
        fi
        
        # Extraer datos del output
        EXEC_TIME=$(echo "$OUTPUT" | grep "Total time:" | awk '{print $3}')
        MEAN_VEL=$(echo "$OUTPUT" | grep "Mean:" | awk '{print $2}')
        TOTAL_CARS=$(echo "$OUTPUT" | grep "Total cars:" | awk '{print $3}')
        
        # Validar que se extrajeron los datos
        if [ -z "$EXEC_TIME" ] || [ -z "$MEAN_VEL" ] || [ -z "$TOTAL_CARS" ]; then
            echo -e "${RED}  Run $run: ERROR - No se pudieron extraer métricas${NC}"
            echo "$OUTPUT" > "${OUTPUT_DIR}/error_${version}_${N}_${P}_${run}.log"
            continue
        fi
        
        # Escribir al CSV
        echo "$version,$N,$T,$density,$P,$run,$EXEC_TIME,$MEAN_VEL,$TOTAL_CARS" >> "$OUTPUT_CSV"
        
        # Progreso
        printf "  Run %2d/%d: %.6fs | velocity=%.4f | cars=%d\n" \
            $run $NUM_RUNS $EXEC_TIME $MEAN_VEL $TOTAL_CARS
    done
    
    echo ""
}

# ============================================
# BENCHMARKS - Configuraciones
# ============================================

# Parámetros comunes
DENSITY=0.5
SEED_BASE=42

# Nota: N debe ser divisible por P
# Para t3.micro cluster (6 procesos max), usamos N divisibles por 1,2,4,6

echo -e "${BLUE}=== FASE 1: Serial Baseline ===${NC}"
echo ""

# Serial - Diferentes tamaños
run_benchmark "serial" "bin/ca_serial" 100000 100 $DENSITY 1 $SEED_BASE
run_benchmark "serial" "bin/ca_serial" 500000 100 $DENSITY 1 $((SEED_BASE+100))
run_benchmark "serial" "bin/ca_serial" 1000000 100 $DENSITY 1 $((SEED_BASE+200))

echo -e "${BLUE}=== FASE 2: MPI Blocking - Strong Scaling ===${NC}"
echo "Fixed N=1200000, vary P"
echo ""

# Strong scaling: N fijo, variar P
N_STRONG=1200000
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" $N_STRONG 100 $DENSITY 1 $((SEED_BASE+300))
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" $N_STRONG 100 $DENSITY 2 $((SEED_BASE+310))
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" $N_STRONG 100 $DENSITY 4 $((SEED_BASE+320))
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" $N_STRONG 100 $DENSITY 6 $((SEED_BASE+330))

echo -e "${BLUE}=== FASE 3: MPI Non-Blocking - Strong Scaling ===${NC}"
echo "Fixed N=1200000, vary P"
echo ""

run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" $N_STRONG 100 $DENSITY 1 $((SEED_BASE+400))
run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" $N_STRONG 100 $DENSITY 2 $((SEED_BASE+410))
run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" $N_STRONG 100 $DENSITY 4 $((SEED_BASE+420))
run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" $N_STRONG 100 $DENSITY 6 $((SEED_BASE+430))

echo -e "${BLUE}=== FASE 4: Weak Scaling (Blocking) ===${NC}"
echo "Fixed local_N=200000, increase N proportional to P"
echo ""

# Weak scaling: N/P constante
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" 200000 100 $DENSITY 1 $((SEED_BASE+500))
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" 400000 100 $DENSITY 2 $((SEED_BASE+510))
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" 800000 100 $DENSITY 4 $((SEED_BASE+520))
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" 1200000 100 $DENSITY 6 $((SEED_BASE+530))

echo -e "${BLUE}=== FASE 5: Weak Scaling (Non-Blocking) ===${NC}"
echo "Fixed local_N=200000, increase N proportional to P"
echo ""

run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" 200000 100 $DENSITY 1 $((SEED_BASE+600))
run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" 400000 100 $DENSITY 2 $((SEED_BASE+610))
run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" 800000 100 $DENSITY 4 $((SEED_BASE+620))
run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" 1200000 100 $DENSITY 6 $((SEED_BASE+630))

echo -e "${BLUE}=== FASE 6: Diferentes Densidades ===${NC}"
echo "Test phase transitions: sparse, medium, dense"
echo ""

# Densidad variable (P=4, N=1000000)
N_DENSITY=1000000
P_DENSITY=4

for DENS in 0.3 0.5 0.7; do
    echo -e "${YELLOW}Density = $DENS${NC}"
    run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" $N_DENSITY 100 $DENS $P_DENSITY $((SEED_BASE+700))
    run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" $N_DENSITY 100 $DENS $P_DENSITY $((SEED_BASE+800))
done

echo -e "${BLUE}=== FASE 7: Muchos Timesteps ===${NC}"
echo "Test with T=1000 (more iterations)"
echo ""

# Más timesteps para ver estabilidad
run_benchmark "mpi_blocking" "bin/ca_mpi_blocking" 600000 1000 $DENSITY 6 $((SEED_BASE+900))
run_benchmark "mpi_nonblocking" "bin/ca_mpi_nonblocking" 600000 1000 $DENSITY 6 $((SEED_BASE+910))

# ============================================
# Resumen
# ============================================

echo "========================================"
echo "Benchmark Completado"
echo "========================================"
echo ""
echo "Resultados guardados en: $OUTPUT_CSV"
echo ""

# Contar líneas (restar header)
TOTAL_TESTS=$(($(wc -l < "$OUTPUT_CSV") - 1))
echo "Total de tests ejecutados: $TOTAL_TESTS"
echo ""

# Preview de los datos
echo "Preview de resultados:"
echo "----------------------------------------"
head -n 11 "$OUTPUT_CSV" | column -t -s,
echo "..."
echo "----------------------------------------"
echo ""

echo -e "${GREEN}✓ Para análisis detallado, ejecuta:${NC}"
echo "  python3 scripts/analyze_results.py $OUTPUT_CSV"
echo ""
echo -e "${GREEN}✓ Para transferir al cluster:${NC}"
echo "  scp $OUTPUT_CSV ubuntu@172.31.28.209:/home/ubuntu/HPC/HPCReto3/results/"
echo ""
