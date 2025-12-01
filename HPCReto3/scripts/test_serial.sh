#!/bin/bash

# Test script for Cellular Automaton Serial Implementation
# Validates correctness with known test cases

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Executable
SERIAL_BIN="./bin/ca_serial"

# Check if binary exists
if [ ! -f "$SERIAL_BIN" ]; then
    echo -e "${RED}Error: Binary not found. Run 'make' first.${NC}"
    exit 1
fi

echo "========================================"
echo "CA Serial Implementation Test Suite"
echo "========================================"
echo ""

# Test counter
PASSED=0
FAILED=0

# Function to run test
run_test() {
    local test_name="$1"
    local N="$2"
    local T="$3"
    local density="$4"
    local seed="$5"
    local expected_behavior="$6"
    
    echo -e "${YELLOW}Test: $test_name${NC}"
    echo "Parameters: N=$N, T=$T, density=$density, seed=$seed"
    
    # Run and capture output
    OUTPUT=$($SERIAL_BIN $N $T $density $seed 2>&1)
    
    # Extract key metrics
    INITIAL_CARS=$(echo "$OUTPUT" | grep "Total cars:" | awk '{print $3}')
    FINAL_CARS=$(echo "$OUTPUT" | grep "Final cars:" | awk '{print $3}')
    CONSERVATION=$(echo "$OUTPUT" | grep "Conservation:" | awk '{print $2}')
    MEAN_VEL=$(echo "$OUTPUT" | grep "Mean:" | awk '{print $2}')
    
    echo "  Initial cars: $INITIAL_CARS"
    echo "  Final cars: $FINAL_CARS"
    echo "  Conservation: $CONSERVATION"
    echo "  Mean velocity: $MEAN_VEL"
    
    # Check conservation
    if [ "$CONSERVATION" == "PASS" ]; then
        echo -e "  ${GREEN}✓ Conservation test passed${NC}"
        ((PASSED++))
    else
        echo -e "  ${RED}✗ Conservation test FAILED${NC}"
        ((FAILED++))
        return 1
    fi
    
    # Check expected behavior
    echo "  Expected: $expected_behavior"
    echo ""
    
    return 0
}

# Test 1: Empty road (density = 0)
# Expected: No cars, velocity undefined (0 or NaN handled gracefully)
run_test "Empty road" 100 10 0.0 42 "No cars, velocity=0 (no movement possible)"

# Test 2: Full road (density = 1.0)
# Expected: All blocked, velocity = 0
run_test "Full road (gridlock)" 100 10 1.0 42 "All cars blocked, velocity=0"

# Test 3: Single car (very sparse)
# Expected: Car moves freely, velocity ≈ 1.0
run_test "Single car" 100 10 0.01 42 "Car moves freely, velocity≈1.0"

# Test 4: Sparse traffic (density = 0.3)
# Expected: High velocity (most cars move)
run_test "Sparse traffic" 200 50 0.3 42 "High velocity (0.7-0.9), free flow"

# Test 5: Medium density (density = 0.5)
# Expected: Mixed flow, moderate velocity
run_test "Medium density" 200 50 0.5 42 "Moderate velocity (0.3-0.7), mixed flow"

# Test 6: Dense traffic (density = 0.7)
# Expected: Low velocity (congestion)
run_test "Dense traffic" 200 50 0.7 42 "Low velocity (0.1-0.4), congestion"

# Test 7: Larger system
# Expected: Should complete without errors
run_test "Large system" 10000 100 0.5 123 "Completes without error, similar velocity to medium density"

# Test 8: Many timesteps
# Expected: Stable behavior over time
run_test "Long simulation" 1000 1000 0.5 456 "Reaches steady state, velocity stabilizes"

# Test 9: Determinism check (same seed, same results)
echo -e "${YELLOW}Test: Determinism${NC}"
echo "Running same configuration twice with same seed..."

OUTPUT1=$($SERIAL_BIN 500 50 0.5 999 2>&1)
CHECKSUM1=$(echo "$OUTPUT1" | grep "Final checksum:" | awk '{print $3}')
MEAN_VEL1=$(echo "$OUTPUT1" | grep "Mean:" | awk '{print $2}')

OUTPUT2=$($SERIAL_BIN 500 50 0.5 999 2>&1)
CHECKSUM2=$(echo "$OUTPUT2" | grep "Final checksum:" | awk '{print $3}')
MEAN_VEL2=$(echo "$OUTPUT2" | grep "Mean:" | awk '{print $2}')

echo "  Run 1: checksum=$CHECKSUM1, mean_vel=$MEAN_VEL1"
echo "  Run 2: checksum=$CHECKSUM2, mean_vel=$MEAN_VEL2"

if [ "$CHECKSUM1" == "$CHECKSUM2" ] && [ "$MEAN_VEL1" == "$MEAN_VEL2" ]; then
    echo -e "  ${GREEN}✓ Determinism test passed${NC}"
    ((PASSED++))
else
    echo -e "  ${RED}✗ Determinism test FAILED${NC}"
    ((FAILED++))
fi
echo ""

# Test 10: Different seeds (should give different results)
echo -e "${YELLOW}Test: Different seeds${NC}"
echo "Running with different seeds..."

OUTPUT_A=$($SERIAL_BIN 500 50 0.5 111 2>&1)
CHECKSUM_A=$(echo "$OUTPUT_A" | grep "Final checksum:" | awk '{print $3}')

OUTPUT_B=$($SERIAL_BIN 500 50 0.5 222 2>&1)
CHECKSUM_B=$(echo "$OUTPUT_B" | grep "Final checksum:" | awk '{print $3}')

echo "  Seed 111: checksum=$CHECKSUM_A"
echo "  Seed 222: checksum=$CHECKSUM_B"

if [ "$CHECKSUM_A" != "$CHECKSUM_B" ]; then
    echo -e "  ${GREEN}✓ Different seeds produce different results${NC}"
    ((PASSED++))
else
    echo -e "  ${RED}✗ Different seeds test FAILED (unlikely but possible)${NC}"
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
