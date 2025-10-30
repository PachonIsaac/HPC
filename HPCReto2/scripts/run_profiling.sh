#!/bin/bash
# Script de Profiling para HPCReto2 - OpenMP Implementations
# Ejecuta gprof y perf para análisis de rendimiento

# Colores para output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  HPCReto2 - Profiling Script${NC}"
echo -e "${BLUE}================================================${NC}"

# Configuración
BIN_DIR="../bin"
RESULTS_DIR="../results/profiling"
TEST_SIZE=10000000  # Tamaño de prueba para profiling
THREADS=4           # Número de threads para profiling

# Crear directorio de resultados
mkdir -p "$RESULTS_DIR"

export OMP_NUM_THREADS=$THREADS

# ==========================================
# Función para profiling con gprof
# ==========================================
run_gprof() {
    local program=$1
    local name=$2
    
    echo -e "\n${GREEN}=== gprof: $name ===${NC}"
    
    if [ ! -f "$program" ]; then
        echo -e "${RED}Error: $program not found${NC}"
        return 1
    fi
    
    # Ejecutar programa (genera gmon.out)
    echo -e "${YELLOW}Running $name...${NC}"
    $program $TEST_SIZE > /dev/null
    
    # Generar reporte gprof
    if [ -f "gmon.out" ]; then
        gprof $program gmon.out > "$RESULTS_DIR/${name}_gprof.txt"
        echo -e "${GREEN}Report saved: $RESULTS_DIR/${name}_gprof.txt${NC}"
        rm gmon.out
    else
        echo -e "${RED}Warning: gmon.out not generated (compile with -pg)${NC}"
    fi
}

# ==========================================
# Función para profiling con perf (Linux only)
# ==========================================
run_perf() {
    local program=$1
    local name=$2
    
    echo -e "\n${GREEN}=== perf: $name ===${NC}"
    
    if [ ! -f "$program" ]; then
        echo -e "${RED}Error: $program not found${NC}"
        return 1
    fi
    
    # Check if perf is available
    if command -v perf &> /dev/null; then
        echo -e "${YELLOW}Running perf stat...${NC}"
        perf stat -o "$RESULTS_DIR/${name}_perf.txt" $program $TEST_SIZE
        echo -e "${GREEN}Report saved: $RESULTS_DIR/${name}_perf.txt${NC}"
    else
        echo -e "${YELLOW}perf not available (Linux tool)${NC}"
    fi
}

# ==========================================
# Profiling de versiones compiladas con -pg
# ==========================================

echo -e "\n${BLUE}Note: Make sure to compile profiling versions with:${NC}"
echo -e "${YELLOW}make profiling${NC}\n"

# Needles profiling
if [ -f "$BIN_DIR/pi_needles_omp_prof" ]; then
    run_gprof "$BIN_DIR/pi_needles_omp_prof" "pi_needles_omp"
    run_perf "$BIN_DIR/pi_needles_omp_prof" "pi_needles_omp"
else
    echo -e "${YELLOW}Warning: pi_needles_omp_prof not found. Run 'make profiling' first.${NC}"
fi

# Dartboard profiling
if [ -f "$BIN_DIR/pi_dartboard_omp_prof" ]; then
    run_gprof "$BIN_DIR/pi_dartboard_omp_prof" "pi_dartboard_omp"
    run_perf "$BIN_DIR/pi_dartboard_omp_prof" "pi_dartboard_omp"
else
    echo -e "${YELLOW}Warning: pi_dartboard_omp_prof not found. Run 'make profiling' first.${NC}"
fi

# ==========================================
# Valgrind cachegrind (si está disponible)
# ==========================================
echo -e "\n${GREEN}=== Cachegrind Analysis ===${NC}"

if command -v valgrind &> /dev/null; then
    # Needles cachegrind
    if [ -f "$BIN_DIR/pi_needles_omp_basic" ]; then
        echo -e "${YELLOW}Running cachegrind for needles...${NC}"
        valgrind --tool=cachegrind --cachegrind-out-file="$RESULTS_DIR/cachegrind_needles.out" \
                 "$BIN_DIR/pi_needles_omp_basic" 1000000 2> "$RESULTS_DIR/cachegrind_needles.txt"
        echo -e "${GREEN}Report saved: $RESULTS_DIR/cachegrind_needles.txt${NC}"
    fi
    
    # Dartboard cachegrind
    if [ -f "$BIN_DIR/pi_dartboard_omp_basic" ]; then
        echo -e "${YELLOW}Running cachegrind for dartboard...${NC}"
        valgrind --tool=cachegrind --cachegrind-out-file="$RESULTS_DIR/cachegrind_dartboard.out" \
                 "$BIN_DIR/pi_dartboard_omp_basic" 1000000 2> "$RESULTS_DIR/cachegrind_dartboard.txt"
        echo -e "${GREEN}Report saved: $RESULTS_DIR/cachegrind_dartboard.txt${NC}"
    fi
else
    echo -e "${YELLOW}valgrind not available${NC}"
fi

# ==========================================
# Resumen
# ==========================================
echo -e "\n${GREEN}================================================${NC}"
echo -e "${GREEN}Profiling Complete!${NC}"
echo -e "${GREEN}================================================${NC}"
echo -e "Reports saved in: ${BLUE}$RESULTS_DIR/${NC}"
echo -e "\nGenerated files:"
ls -lh "$RESULTS_DIR/" 2>/dev/null | tail -n +2 | awk '{print "  - " $9 " (" $5 ")"}'

echo -e "\n${YELLOW}Next steps:${NC}"
echo -e "  1. Review gprof reports: ${BLUE}less $RESULTS_DIR/*_gprof.txt${NC}"
echo -e "  2. Review perf reports: ${BLUE}less $RESULTS_DIR/*_perf.txt${NC}"
echo -e "  3. Analyze cachegrind: ${BLUE}cg_annotate $RESULTS_DIR/cachegrind_*.out${NC}"
