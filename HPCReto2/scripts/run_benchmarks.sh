#!/bin/bash
# Script de Benchmarking para HPCReto2 - OpenMP Implementations
# Ejecuta pruebas con diferentes configuraciones y guarda resultados

# Colores para output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  HPCReto2 - OpenMP Benchmarking Script${NC}"
echo -e "${BLUE}================================================${NC}"

# Configuración
BIN_DIR="../bin"
RESULTS_DIR="../results"
RESULTS_FILE="$RESULTS_DIR/benchmarks.csv"
SPEEDUP_FILE="$RESULTS_DIR/speedup.csv"

# Parámetros de prueba
SIZES=(1000000 10000000 100000000)  # Tamaños de problema
THREADS=(1 2 4 8 16)                # Números de threads
RUNS=5                               # Ejecuciones por configuración

# Crear directorio de resultados si no existe
mkdir -p "$RESULTS_DIR"

# Inicializar archivo CSV
echo "Algorithm,Version,Iterations,Threads,Run,PI_Estimate,Time_Seconds" > "$RESULTS_FILE"

# Función para ejecutar un programa y extraer tiempo
run_benchmark() {
    local program=$1
    local iterations=$2
    local threads=$3
    local algorithm=$4
    local version=$5
    
    export OMP_NUM_THREADS=$threads
    
    for run in $(seq 1 $RUNS); do
        echo -e "${YELLOW}  [Run $run/$RUNS]${NC} $algorithm - $version - Iter: $iterations - Threads: $threads"
        
        # Ejecutar y capturar output
        output=$($program $iterations 2>&1)
        
        # Extraer PI estimado y tiempo
        pi_estimate=$(echo "$output" | grep "PI estimado" | awk -F'= ' '{print $2}')
        time_seconds=$(echo "$output" | grep "Tiempo de ejecución" | awk '{print $4}')
        
        # Guardar en CSV
        echo "$algorithm,$version,$iterations,$threads,$run,$pi_estimate,$time_seconds" >> "$RESULTS_FILE"
    done
}

# ==========================================
# NEEDLES - Serial Baseline
# ==========================================
echo -e "\n${GREEN}=== Buffon's Needle - Serial ===${NC}"
if [ -f "$BIN_DIR/pi_needles_serial" ]; then
    for size in "${SIZES[@]}"; do
        echo -e "${BLUE}Size: $size${NC}"
        run_benchmark "$BIN_DIR/pi_needles_serial" $size 1 "Needles" "Serial"
    done
else
    echo -e "${YELLOW}Warning: pi_needles_serial not found${NC}"
fi

# ==========================================
# NEEDLES - OpenMP Basic
# ==========================================
echo -e "\n${GREEN}=== Buffon's Needle - OpenMP Basic ===${NC}"
if [ -f "$BIN_DIR/pi_needles_omp_basic" ]; then
    for size in "${SIZES[@]}"; do
        echo -e "${BLUE}Size: $size${NC}"
        for thread in "${THREADS[@]}"; do
            run_benchmark "$BIN_DIR/pi_needles_omp_basic" $size $thread "Needles" "OMP_Basic"
        done
    done
else
    echo -e "${YELLOW}Warning: pi_needles_omp_basic not found${NC}"
fi

# ==========================================
# NEEDLES - OpenMP Scheduling Policies
# ==========================================
for sched in static dynamic guided; do
    echo -e "\n${GREEN}=== Buffon's Needle - OpenMP $sched ===${NC}"
    binary="$BIN_DIR/pi_needles_omp_$sched"
    
    if [ -f "$binary" ]; then
        for size in "${SIZES[@]}"; do
            echo -e "${BLUE}Size: $size${NC}"
            for thread in "${THREADS[@]}"; do
                run_benchmark "$binary" $size $thread "Needles" "OMP_${sched^}"
            done
        done
    else
        echo -e "${YELLOW}Warning: $binary not found${NC}"
    fi
done

# ==========================================
# NEEDLES - OpenMP Optimized
# ==========================================
echo -e "\n${GREEN}=== Buffon's Needle - OpenMP Optimized ===${NC}"
if [ -f "$BIN_DIR/pi_needles_omp_optimized" ]; then
    for size in "${SIZES[@]}"; do
        echo -e "${BLUE}Size: $size${NC}"
        for thread in "${THREADS[@]}"; do
            run_benchmark "$BIN_DIR/pi_needles_omp_optimized" $size $thread "Needles" "OMP_Optimized"
        done
    done
else
    echo -e "${YELLOW}Warning: pi_needles_omp_optimized not found${NC}"
fi

# ==========================================
# DARTBOARD - Serial Baseline
# ==========================================
echo -e "\n${GREEN}=== Dartboard - Serial ===${NC}"
if [ -f "$BIN_DIR/pi_dartboard_serial" ]; then
    for size in "${SIZES[@]}"; do
        echo -e "${BLUE}Size: $size${NC}"
        run_benchmark "$BIN_DIR/pi_dartboard_serial" $size 1 "Dartboard" "Serial"
    done
else
    echo -e "${YELLOW}Warning: pi_dartboard_serial not found${NC}"
fi

# ==========================================
# DARTBOARD - OpenMP Basic
# ==========================================
echo -e "\n${GREEN}=== Dartboard - OpenMP Basic ===${NC}"
if [ -f "$BIN_DIR/pi_dartboard_omp_basic" ]; then
    for size in "${SIZES[@]}"; do
        echo -e "${BLUE}Size: $size${NC}"
        for thread in "${THREADS[@]}"; do
            run_benchmark "$BIN_DIR/pi_dartboard_omp_basic" $size $thread "Dartboard" "OMP_Basic"
        done
    done
else
    echo -e "${YELLOW}Warning: pi_dartboard_omp_basic not found${NC}"
fi

# ==========================================
# DARTBOARD - OpenMP Scheduling Policies
# ==========================================
for sched in static dynamic guided; do
    echo -e "\n${GREEN}=== Dartboard - OpenMP $sched ===${NC}"
    binary="$BIN_DIR/pi_dartboard_omp_$sched"
    
    if [ -f "$binary" ]; then
        for size in "${SIZES[@]}"; do
            echo -e "${BLUE}Size: $size${NC}"
            for thread in "${THREADS[@]}"; do
                run_benchmark "$binary" $size $thread "Dartboard" "OMP_${sched^}"
            done
        done
    else
        echo -e "${YELLOW}Warning: $binary not found${NC}"
    fi
done

# ==========================================
# DARTBOARD - OpenMP Optimized
# ==========================================
echo -e "\n${GREEN}=== Dartboard - OpenMP Optimized ===${NC}"
if [ -f "$BIN_DIR/pi_dartboard_omp_optimized" ]; then
    for size in "${SIZES[@]}"; do
        echo -e "${BLUE}Size: $size${NC}"
        for thread in "${THREADS[@]}"; do
            run_benchmark "$BIN_DIR/pi_dartboard_omp_optimized" $size $thread "Dartboard" "OMP_Optimized"
        done
    done
else
    echo -e "${YELLOW}Warning: pi_dartboard_omp_optimized not found${NC}"
fi

# ==========================================
# Calcular Speedup y Eficiencia
# ==========================================
echo -e "\n${GREEN}=== Calculating Speedup and Efficiency ===${NC}"

# Crear archivo de speedup
echo "Algorithm,Version,Iterations,Threads,Avg_Time_Serial,Avg_Time_Parallel,Speedup,Efficiency" > "$SPEEDUP_FILE"

# Procesar resultados con Python/awk (simplificado aquí)
echo "Speedup calculation would be processed here..."
echo "Use Python/R script for detailed analysis"

echo -e "\n${GREEN}================================================${NC}"
echo -e "${GREEN}Benchmarking Complete!${NC}"
echo -e "${GREEN}================================================${NC}"
echo -e "Results saved to:"
echo -e "  - ${BLUE}$RESULTS_FILE${NC}"
echo -e "  - ${BLUE}$SPEEDUP_FILE${NC}"
echo -e "\nNext steps:"
echo -e "  1. Analyze results: ${YELLOW}cat $RESULTS_FILE${NC}"
echo -e "  2. Run profiling: ${YELLOW}./run_profiling.sh${NC}"
echo -e "  3. Generate plots: ${YELLOW}python3 ../scripts/plot_results.py${NC}"
