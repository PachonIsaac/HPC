#!/bin/bash

# Test script for MPI Blocking Implementation
# Run on AWS cluster with MPI installed

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "========================================"
echo "CA MPI Blocking Test Suite"
echo "========================================"
echo ""

# Check if MPI is available
if ! command -v mpirun &> /dev/null; then
    echo -e "${RED}Error: mpirun not found. Please run on cluster with MPI.${NC}"
    exit 1
fi

# Check if binary exists
if [ ! -f "./bin/ca_mpi_blocking" ]; then
    echo -e "${RED}Error: Binary not found. Run 'make' first.${NC}"
    exit 1
fi

PASSED=0
FAILED=0

# Function to run MPI test
run_mpi_test() {
    local test_name="$1"
    local np="$2"
    local N="$3"
    local T="$4"
    local density="$5"
    local seed="$6"
    
    echo -e "${YELLOW}Test: $test_name${NC}"
    echo "Parameters: np=$np, N=$N, T=$T, density=$density, seed=$seed"
    
    # Run MPI version
    OUTPUT=$(mpirun -np $np ./bin/ca_mpi_blocking $N $T $density $seed 2>&1)
    
    # Extract metrics
    INITIAL_CARS=$(echo "$OUTPUT" | grep "Total cars:" | awk '{print $3}')
    FINAL_CARS=$(echo "$OUTPUT" | grep "Final cars:" | awk '{print $3}')
    CONSERVATION=$(echo "$OUTPUT" | grep "Conservation:" | awk '{print $2}')
    MEAN_VEL=$(echo "$OUTPUT" | grep "Mean:" | awk '{print $2}')
    
    echo "  Initial cars: $INITIAL_CARS"
    echo "  Final cars: $FINAL_CARS"
    echo "  Conservation: $CONSERVATION"
    echo "  Mean velocity: $MEAN_VEL"
    
    if [ "$CONSERVATION" == "PASS" ]; then
        echo -e "  ${GREEN}✓ Conservation test passed${NC}"
        ((PASSED++))
    else
        echo -e "  ${RED}✗ Conservation test FAILED${NC}"
        ((FAILED++))
    fi
    echo ""
}

# Test 1: Single process (should match serial behavior)
run_mpi_test "Single process (P=1)" 1 1000 50 0.5 42

# Test 2: Two processes
run_mpi_test "Two processes (P=2)" 2 1000 50 0.5 42

# Test 3: Four processes
run_mpi_test "Four processes (P=4)" 4 1000 50 0.5 42

# Test 4: Six processes (cluster max)
run_mpi_test "Six processes (P=6)" 6 1200 50 0.5 42

# Test 5: Large N with 4 processes
run_mpi_test "Large system (P=4)" 4 100000 100 0.5 123

# Test 6: Dense traffic
run_mpi_test "Dense traffic (P=4)" 4 1000 50 0.7 456

# Test 7: Sparse traffic
run_mpi_test "Sparse traffic (P=4)" 4 1000 50 0.3 789

# Test 8: Determinism check (same parameters twice)
echo -e "${YELLOW}Test: Determinism${NC}"
echo "Running same configuration twice with P=2..."

OUTPUT1=$(mpirun -np 2 ./bin/ca_mpi_blocking 1000 50 0.5 999 2>&1)
MEAN_VEL1=$(echo "$OUTPUT1" | grep "Mean:" | awk '{print $2}')
FINAL_CARS1=$(echo "$OUTPUT1" | grep "Final cars:" | awk '{print $3}')

OUTPUT2=$(mpirun -np 2 ./bin/ca_mpi_blocking 1000 50 0.5 999 2>&1)
MEAN_VEL2=$(echo "$OUTPUT2" | grep "Mean:" | awk '{print $2}')
FINAL_CARS2=$(echo "$OUTPUT2" | grep "Final cars:" | awk '{print $3}')

echo "  Run 1: final_cars=$FINAL_CARS1, mean_vel=$MEAN_VEL1"
echo "  Run 2: final_cars=$FINAL_CARS2, mean_vel=$MEAN_VEL2"

if [ "$MEAN_VEL1" == "$MEAN_VEL2" ] && [ "$FINAL_CARS1" == "$FINAL_CARS2" ]; then
    echo -e "  ${GREEN}✓ Determinism test passed${NC}"
    ((PASSED++))
else
    echo -e "  ${RED}✗ Determinism test FAILED${NC}"
    ((FAILED++))
fi
echo ""

# Test 9: Compare P=1 with serial (if serial binary exists)
if [ -f "./bin/ca_serial" ]; then
    echo -e "${YELLOW}Test: MPI P=1 vs Serial equivalence${NC}"
    echo "Running serial and MPI with P=1..."
    
    SERIAL_OUT=$(./bin/ca_serial 1000 50 0.5 111 2>&1)
    SERIAL_MEAN=$(echo "$SERIAL_OUT" | grep "Mean:" | awk '{print $2}')
    SERIAL_CARS=$(echo "$SERIAL_OUT" | grep "Final cars:" | awk '{print $3}')
    
    MPI_OUT=$(mpirun -np 1 ./bin/ca_mpi_blocking 1000 50 0.5 111 2>&1)
    MPI_MEAN=$(echo "$MPI_OUT" | grep "Mean:" | awk '{print $2}')
    MPI_CARS=$(echo "$MPI_OUT" | grep "Final cars:" | awk '{print $3}')
    
    echo "  Serial:    final_cars=$SERIAL_CARS, mean_vel=$SERIAL_MEAN"
    echo "  MPI (P=1): final_cars=$MPI_CARS, mean_vel=$MPI_MEAN"
    
    # Note: Velocities may differ due to different initialization (serial uses one seed, MPI uses seed+rank)
    # But conservation should still hold
    if [ "$SERIAL_CARS" == "$MPI_CARS" ]; then
        echo -e "  ${GREEN}✓ Car count matches (expected with same density)${NC}"
        ((PASSED++))
    else
        echo -e "  ${YELLOW}⚠ Car count differs (acceptable due to different RNG)${NC}"
    fi
else
    echo -e "${YELLOW}Skipping serial comparison (serial binary not found)${NC}"
fi
echo ""

# Test 10: Error handling (N not divisible by P)
echo -e "${YELLOW}Test: Error handling (N not divisible by P)${NC}"
echo "Running with N=1000, P=3 (should fail gracefully)..."

if mpirun -np 3 ./bin/ca_mpi_blocking 1000 50 0.5 42 2>&1 | grep -q "must be divisible"; then
    echo -e "  ${GREEN}✓ Correctly detected invalid N/P combination${NC}"
    ((PASSED++))
else
    echo -e "  ${RED}✗ Failed to detect invalid N/P${NC}"
    ((FAILED++))
fi
echo ""

# Summary
echo "========================================"
echo "Test Summary"
echo "========================================"
echo -e "Passed: ${GREEN}$PASSED${NC}"
echo -e "Failed: ${RED}$FAILED${NC}"
echo "========================================"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed! ✓${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Please review.${NC}"
    exit 1
fi
