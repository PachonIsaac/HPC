#!/bin/bash
# Simplified Benchmark Script - Para pruebas más rápidas
# Ejecuta solo las versiones principales con menos repeticiones

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  HPCReto2 - Simplified Benchmarking${NC}"
echo -e "${BLUE}================================================${NC}"

BIN_DIR="../bin"
RESULTS_DIR="../results"
RESULTS_FILE="$RESULTS_DIR/benchmarks.csv"

# Configuración reducida para pruebas
SIZES=(1000000 10000000)  # Solo 2 tamaños
THREADS=(1 2 4 8)          # 4 configuraciones de threads
RUNS=3                     # 3 ejecuciones por configuración

mkdir -p "$RESULTS_DIR"

# Inicializar CSV
echo "Algorithm,Version,Iterations,Threads,Run,PI_Estimate,Time_Seconds" > "$RESULTS_FILE"

# Función para ejecutar y guardar
run_benchmark() {
    local program=$1
    local iterations=$2
    local threads=$3
    local algorithm=$4
    local version=$5
    
    export OMP_NUM_THREADS=$threads
    
    for run in $(seq 1 $RUNS); do
        echo -e "${YELLOW}  [$run/$RUNS]${NC} $algorithm - $version - Iter: $iterations - Threads: $threads"
        
        output=$($program $iterations 2>&1)
        
        pi_estimate=$(echo "$output" | grep "PI estimado" | awk -F'= ' '{print $2}')
        time_seconds=$(echo "$output" | grep "Tiempo de ejecución" | awk '{print $4}')
        
        echo "$algorithm,$version,$iterations,$threads,$run,$pi_estimate,$time_seconds" >> "$RESULTS_FILE"
    done
}

# Versiones a probar (pares version:binary)

# ===== NEEDLES =====
echo -e "\n${GREEN}=== Buffon's Needle ===${NC}"
for size in "${SIZES[@]}"; do
    echo -e "\n${BLUE}Size: $size${NC}"
    
    # Serial
    if [ -f "$BIN_DIR/pi_needles_serial" ]; then
        run_benchmark "$BIN_DIR/pi_needles_serial" $size 1 "Needles" "Serial"
    fi
    
    # Versiones paralelas
    if [ -f "$BIN_DIR/pi_needles_omp_basic" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_needles_omp_basic" $size $thread "Needles" "OMP_Basic"
        done
    fi
    
    if [ -f "$BIN_DIR/pi_needles_omp_static" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_needles_omp_static" $size $thread "Needles" "OMP_Static"
        done
    fi
    
    if [ -f "$BIN_DIR/pi_needles_omp_dynamic" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_needles_omp_dynamic" $size $thread "Needles" "OMP_Dynamic"
        done
    fi
    
    if [ -f "$BIN_DIR/pi_needles_omp_guided" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_needles_omp_guided" $size $thread "Needles" "OMP_Guided"
        done
    fi
    
    if [ -f "$BIN_DIR/pi_needles_omp_optimized" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_needles_omp_optimized" $size $thread "Needles" "OMP_Optimized"
        done
    fi
done

# ===== DARTBOARD =====
echo -e "\n${GREEN}=== Monte Carlo Dartboard ===${NC}"
for size in "${SIZES[@]}"; do
    echo -e "\n${BLUE}Size: $size${NC}"
    
    # Serial
    if [ -f "$BIN_DIR/pi_dartboard_serial" ]; then
        run_benchmark "$BIN_DIR/pi_dartboard_serial" $size 1 "Dartboard" "Serial"
    fi
    
    # Versiones paralelas
    if [ -f "$BIN_DIR/pi_dartboard_omp_basic" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_dartboard_omp_basic" $size $thread "Dartboard" "OMP_Basic"
        done
    fi
    
    if [ -f "$BIN_DIR/pi_dartboard_omp_static" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_dartboard_omp_static" $size $thread "Dartboard" "OMP_Static"
        done
    fi
    
    if [ -f "$BIN_DIR/pi_dartboard_omp_dynamic" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_dartboard_omp_dynamic" $size $thread "Dartboard" "OMP_Dynamic"
        done
    fi
    
    if [ -f "$BIN_DIR/pi_dartboard_omp_guided" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_dartboard_omp_guided" $size $thread "Dartboard" "OMP_Guided"
        done
    fi
    
    if [ -f "$BIN_DIR/pi_dartboard_omp_optimized" ]; then
        for thread in "${THREADS[@]}"; do
            [ $thread -eq 1 ] && continue
            run_benchmark "$BIN_DIR/pi_dartboard_omp_optimized" $size $thread "Dartboard" "OMP_Optimized"
        done
    fi
done

echo -e "\n${GREEN}================================================${NC}"
echo -e "${GREEN}Benchmarking Complete!${NC}"
echo -e "${GREEN}================================================${NC}"
echo -e "Results saved to: ${BLUE}$RESULTS_FILE${NC}"
echo -e "\nNext steps:"
echo -e "  1. Analyze: ${YELLOW}python3 analyze_benchmarks.py $RESULTS_FILE${NC}"
echo -e "  2. Visualize: ${YELLOW}python3 plot_results.py ../results/speedup_analysis.csv${NC}"
