#!/bin/bash
# test_quick.sh - Test rápido de las implementaciones MPI localmente

echo "=== Quick MPI Test ==="
echo "Testing locally with small matrix"
echo ""

MATRIX_SIZE=256
NUM_PROCS=2

# Compilar
echo "Building..."
cd /Users/isaacpachon/Desktop/Dev/UTP/HPC/HPCCasoEstudio3
make clean > /dev/null 2>&1
make

if [ $? -ne 0 ]; then
    echo "❌ Compilation failed"
    exit 1
fi

echo "✓ Compilation successful"
echo ""

# Test cada implementación
IMPLEMENTATIONS=("sequential" "rowwise" "broadcast" "nonblocking")

for impl in "${IMPLEMENTATIONS[@]}"; do
    echo "Testing: $impl"
    binary="./bin/matrix_mpi_${impl}"
    
    if [ ! -f "$binary" ]; then
        echo "  ❌ Binary not found: $binary"
        continue
    fi
    
    output=$(mpirun -np $NUM_PROCS $binary $MATRIX_SIZE 2>&1)
    exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        time=$(echo "$output" | grep -i "time" | head -1 | awk '{print $3}')
        echo "  ✓ Success"
        echo "$output" | grep -E "(Matrix size|Number of|time|Sample)"
    else
        echo "  ❌ Failed (exit code: $exit_code)"
        echo "$output" | head -5
    fi
    echo ""
done

echo "=== Quick Test Complete ==="
echo ""
echo "If all tests passed, you're ready to:"
echo "  1. Deploy to cluster: ./scripts/deploy.sh"
echo "  2. Run full benchmarks: ./scripts/run_benchmarks.sh"
