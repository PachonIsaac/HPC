# Makefile para HPCCasoEstudio3 - MPI Matrix Multiplication
# Multiplicación de Matrices Distribuida con MPI

MPICC = mpicc
CFLAGS = -Wall -O2
LDFLAGS = -lm

SRC = src
BIN = bin
RESULTS = results

# Targets
TARGETS = matrix_mpi_sequential matrix_mpi_rowwise matrix_mpi_broadcast matrix_mpi_nonblocking

.PHONY: all clean help

all: $(TARGETS)

# Versión Sequential (Baseline - no usa MPI para multiplicación)
matrix_mpi_sequential: $(SRC)/matrix_mpi_sequential.c
	$(MPICC) $(CFLAGS) -o $(BIN)/$@ $< $(LDFLAGS)

# Versión Row-wise Distribution (Master-Worker básico)
matrix_mpi_rowwise: $(SRC)/matrix_mpi_rowwise.c
	$(MPICC) $(CFLAGS) -o $(BIN)/$@ $< $(LDFLAGS)

# Versión Broadcast Optimizado (Broadcast B completa)
matrix_mpi_broadcast: $(SRC)/matrix_mpi_broadcast.c
	$(MPICC) $(CFLAGS) -o $(BIN)/$@ $< $(LDFLAGS)

# Versión Non-blocking Communication
matrix_mpi_nonblocking: $(SRC)/matrix_mpi_nonblocking.c
	$(MPICC) $(CFLAGS) -o $(BIN)/$@ $< $(LDFLAGS)

clean:
	rm -f $(BIN)/*
	rm -f $(RESULTS)/*.txt

clean-all: clean
	rm -f $(RESULTS)/*.csv
	rm -f $(RESULTS)/*.dat

help:
	@echo "=== HPCCasoEstudio3 Makefile ==="
	@echo "Targets disponibles:"
	@echo "  make all                    - Compila todas las versiones"
	@echo "  make matrix_mpi_sequential  - Baseline (serial con framework MPI)"
	@echo "  make matrix_mpi_rowwise     - Row-wise distribution"
	@echo "  make matrix_mpi_broadcast   - Broadcast optimizado"
	@echo "  make matrix_mpi_nonblocking - Non-blocking communication"
	@echo "  make clean                  - Limpia binarios"
	@echo "  make clean-all              - Limpia todo incluyendo resultados"
	@echo ""
	@echo "Ejecución en cluster:"
	@echo "  mpirun -np 4 -hostfile ~/mpi_hosts ./bin/matrix_mpi_rowwise 512"
