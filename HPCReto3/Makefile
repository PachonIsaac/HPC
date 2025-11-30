# Makefile for Cellular Automaton Traffic Simulation
# Author: Isaac Pachon
# Course: HPC - Reto 3

# Compiler and flags
CC = gcc
MPICC = mpicc
CFLAGS = -O2 -Wall -Wextra
LDFLAGS = -lm

# Directories
SRC_DIR = src
BIN_DIR = bin

# Executables
SERIAL = $(BIN_DIR)/ca_serial
MPI_BLOCKING = $(BIN_DIR)/ca_mpi_blocking
MPI_NONBLOCKING = $(BIN_DIR)/ca_mpi_nonblocking
MPI_MASTER_WORKER = $(BIN_DIR)/ca_mpi_master_worker

# All targets
ALL_TARGETS = $(SERIAL) $(MPI_BLOCKING) $(MPI_NONBLOCKING)
# When master-worker is ready, add:
# ALL_TARGETS = $(SERIAL) $(MPI_BLOCKING) $(MPI_NONBLOCKING) $(MPI_MASTER_WORKER)

# Default target
.PHONY: all
all: $(BIN_DIR) $(ALL_TARGETS)

# Create bin directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Serial version
$(SERIAL): $(SRC_DIR)/ca_serial.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Built: $@"

# MPI blocking version (to be implemented)
$(MPI_BLOCKING): $(SRC_DIR)/ca_mpi_blocking.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Built: $@"

# MPI non-blocking version (to be implemented)
$(MPI_NONBLOCKING): $(SRC_DIR)/ca_mpi_nonblocking.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Built: $@"

# MPI master-worker version (to be implemented)
$(MPI_MASTER_WORKER): $(SRC_DIR)/ca_mpi_master_worker.c
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Built: $@"

# Clean
.PHONY: clean
clean:
	rm -rf $(BIN_DIR)
	rm -f results/*.csv results/*.txt
	@echo "Cleaned build artifacts and results"

# Test serial version with various scenarios
.PHONY: test-serial
test-serial: $(SERIAL)
	@echo "Testing serial version..."
	@echo "\n=== Test 1: Empty road ==="
	$(SERIAL) 20 10 0.0 123
	@echo "\n=== Test 2: Full road (gridlock) ==="
	$(SERIAL) 20 10 1.0 123
	@echo "\n=== Test 3: Single car ==="
	$(SERIAL) 20 10 0.05 123
	@echo "\n=== Test 4: Alternating pattern ==="
	# Manual test needed for specific pattern
	@echo "\n=== Test 5: Sparse traffic ==="
	$(SERIAL) 50 20 0.3 123
	@echo "\n=== Test 6: Dense traffic ==="
	$(SERIAL) 50 20 0.7 123
	@echo "\nAll tests completed!"

# Quick benchmark
.PHONY: benchmark-quick
benchmark-quick: $(SERIAL)
	@echo "Quick benchmark (serial)..."
	@echo "N=1000, T=100, density=0.5"
	$(SERIAL) 1000 100 0.5 42
	@echo "\nN=10000, T=100, density=0.5"
	$(SERIAL) 10000 100 0.5 42

# Help
.PHONY: help
help:
	@echo "Makefile for Cellular Automaton Traffic Simulation"
	@echo ""
	@echo "Targets:"
	@echo "  all              - Build all executables (default)"
	@echo "  $(SERIAL)        - Build serial version"
	@echo "  test-serial      - Run test cases for serial version"
	@echo "  benchmark-quick  - Run quick benchmark"
	@echo "  clean            - Remove build artifacts"
	@echo "  help             - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make                    # Build all"
	@echo "  make test-serial        # Test serial version"
	@echo "  make benchmark-quick    # Quick performance test"
	@echo "  ./bin/ca_serial 1000 100 0.5 42  # Custom run"
