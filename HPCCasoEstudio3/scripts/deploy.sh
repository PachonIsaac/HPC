#!/bin/bash
# deploy.sh - Despliega binarios compilados a los nodos workers del cluster

WORKERS=("172.31.18.36" "172.31.16.182")
BIN_DIR="./bin"
REMOTE_DIR="~/HPCCasoEstudio3/bin"

echo "=== Deploying MPI Binaries to Cluster ==="
echo "Compilation host: $(hostname)"
echo "Workers: ${WORKERS[@]}"
echo ""

# Verificar que existen los binarios
if [ ! -d "$BIN_DIR" ]; then
    echo "Error: bin/ directory not found. Run 'make' first."
    exit 1
fi

BINARIES=$(ls $BIN_DIR/matrix_mpi_* 2>/dev/null)
if [ -z "$BINARIES" ]; then
    echo "Error: No MPI binaries found in $BIN_DIR"
    echo "Run 'make' to compile first."
    exit 1
fi

echo "Found binaries:"
for bin in $BINARIES; do
    echo "  - $(basename $bin)"
done
echo ""

# Crear directorio remoto en workers
echo "Creating remote directories..."
for worker in "${WORKERS[@]}"; do
    echo "  $worker: $REMOTE_DIR"
    ssh $worker "mkdir -p $REMOTE_DIR" 2>/dev/null
done
echo ""

# Copiar binarios a cada worker
echo "Copying binaries..."
for worker in "${WORKERS[@]}"; do
    echo "Deploying to $worker..."
    for bin in $BINARIES; do
        echo "  - $(basename $bin)"
        scp -q $bin $worker:$REMOTE_DIR/
        if [ $? -eq 0 ]; then
            echo "    ✓ Success"
        else
            echo "    ✗ Failed"
        fi
    done
    echo ""
done

echo "=== Deployment Complete ==="
echo ""
echo "To verify deployment, run:"
echo "  ssh worker1 'ls -lh ~/HPCCasoEstudio3/bin/'"
echo ""
echo "To run tests:"
echo "  ./scripts/run_benchmarks.sh"
