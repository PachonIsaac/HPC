#!/bin/bash
# Quick Benchmark Test - Prueba rápida de todas las implementaciones

GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}================================================${NC}"
echo -e "${BLUE}  Quick Benchmark Test - HPCReto2${NC}"
echo -e "${BLUE}================================================${NC}"

BIN_DIR="../bin"
TEST_SIZE=1000000
THREADS=(1 2 4 8)

echo -e "\n${GREEN}Test Size: $TEST_SIZE iterations${NC}"
echo -e "${GREEN}Threads: ${THREADS[@]}${NC}\n"

# Test Needles versions
echo -e "${YELLOW}=== NEEDLES ===${NC}"
echo "Serial:"
$BIN_DIR/pi_needles_serial $TEST_SIZE

for t in "${THREADS[@]}"; do
    export OMP_NUM_THREADS=$t
    echo -e "\nOMP Basic ($t threads):"
    $BIN_DIR/pi_needles_omp_basic $TEST_SIZE
done

for t in "${THREADS[@]}"; do
    export OMP_NUM_THREADS=$t
    echo -e "\nOMP Optimized ($t threads):"
    $BIN_DIR/pi_needles_omp_optimized $TEST_SIZE
done

# Test Dartboard versions
echo -e "\n\n${YELLOW}=== DARTBOARD ===${NC}"
echo "Serial:"
$BIN_DIR/pi_dartboard_serial $TEST_SIZE

for t in "${THREADS[@]}"; do
    export OMP_NUM_THREADS=$t
    echo -e "\nOMP Basic ($t threads):"
    $BIN_DIR/pi_dartboard_omp_basic $TEST_SIZE
done

for t in "${THREADS[@]}"; do
    export OMP_NUM_THREADS=$t
    echo -e "\nOMP Optimized ($t threads):"
    $BIN_DIR/pi_dartboard_omp_optimized $TEST_SIZE
done

echo -e "\n${GREEN}================================================${NC}"
echo -e "${GREEN}Quick test completed!${NC}"
echo -e "${GREEN}================================================${NC}"
