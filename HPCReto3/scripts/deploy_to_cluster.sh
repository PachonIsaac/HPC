#!/bin/bash

# Deployment script for AWS cluster
# Compiles and tests MPI versions on the cluster

set -e

CLUSTER_HEAD="172.31.28.209"
CLUSTER_USER="ubuntu"  # Adjust if different
PROJECT_DIR="/home/ubuntu/HPC/HPCReto3"

echo "========================================"
echo "HPCReto3 - AWS Cluster Deployment"
echo "========================================"
echo ""

# Check if we can reach the cluster
echo "Checking cluster connectivity..."
if ! ping -c 1 -W 2 $CLUSTER_HEAD &> /dev/null; then
    echo "Error: Cannot reach cluster head node at $CLUSTER_HEAD"
    echo "Make sure you're connected to the AWS VPN/network"
    exit 1
fi

echo "✓ Cluster reachable"
echo ""

# Sync code to cluster
echo "Syncing code to cluster..."
rsync -avz --exclude 'bin/' --exclude '.git/' --exclude 'results/' \
    ./ ${CLUSTER_USER}@${CLUSTER_HEAD}:${PROJECT_DIR}/

echo "✓ Code synced"
echo ""

# Compile on cluster
echo "Compiling on cluster..."
ssh ${CLUSTER_USER}@${CLUSTER_HEAD} << 'EOF'
cd /home/ubuntu/HPC/HPCReto3
make clean
make
echo ""
echo "Build complete. Binaries:"
ls -lh bin/
EOF

echo ""
echo "========================================"
echo "Deployment complete!"
echo "========================================"
echo ""
echo "To run tests on cluster:"
echo "  ssh ${CLUSTER_USER}@${CLUSTER_HEAD}"
echo "  cd ${PROJECT_DIR}"
echo "  ./scripts/test_mpi_blocking.sh"
echo ""
echo "To run manual tests:"
echo "  mpirun -np 4 --hostfile hostfile ./bin/ca_mpi_blocking 10000 100 0.5 42"
echo ""
