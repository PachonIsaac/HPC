# Makefile para HPCReto2 - OpenMP Implementations
# Algoritmos Monte Carlo para cálculo de PI

CC = gcc
CFLAGS_BASE = -Wall -Wextra

# Detección de sistema operativo y configuración de OpenMP
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS con Apple Clang - necesita libomp
    HOMEBREW_PREFIX := $(shell brew --prefix 2>/dev/null || echo /usr/local)
    LIBOMP_PREFIX := $(shell brew --prefix libomp 2>/dev/null || echo $(HOMEBREW_PREFIX))
    CFLAGS_OMP_FLAGS = -Xpreprocessor -fopenmp -I$(LIBOMP_PREFIX)/include
    LDFLAGS_OMP = -L$(LIBOMP_PREFIX)/lib -lomp
else
    # Linux con gcc
    CFLAGS_OMP_FLAGS = -fopenmp
    LDFLAGS_OMP = -fopenmp
endif

CFLAGS_SERIAL = $(CFLAGS_BASE) -O2
CFLAGS_OMP = $(CFLAGS_BASE) $(CFLAGS_OMP_FLAGS) -O2
CFLAGS_OMP_OPT = $(CFLAGS_BASE) $(CFLAGS_OMP_FLAGS) -O3 -march=native -funroll-loops
CFLAGS_PROF = $(CFLAGS_BASE) $(CFLAGS_OMP_FLAGS) -O2 -pg
LDFLAGS = -lm

SRC = src
BIN = bin
RESULTS = results

# Targets principales
SERIAL_TARGETS = pi_needles_serial pi_dartboard_serial
OMP_BASIC_TARGETS = pi_needles_omp_basic pi_dartboard_omp_basic
OMP_SCHED_TARGETS = pi_needles_omp_static pi_needles_omp_dynamic pi_needles_omp_guided \
                    pi_dartboard_omp_static pi_dartboard_omp_dynamic pi_dartboard_omp_guided
OMP_OPT_TARGETS = pi_needles_omp_optimized pi_dartboard_omp_optimized
PROF_TARGETS = pi_needles_omp_prof pi_dartboard_omp_prof

ALL_TARGETS = $(SERIAL_TARGETS) $(OMP_BASIC_TARGETS) $(OMP_SCHED_TARGETS) $(OMP_OPT_TARGETS)

# Target principal
.PHONY: all
all: $(ALL_TARGETS)

# Versiones Seriales (baseline)
pi_needles_serial: $(SRC)/pi_needles_serial.c | $(BIN)
	$(CC) $(CFLAGS_SERIAL) -o $(BIN)/$@ $< $(LDFLAGS)

pi_dartboard_serial: $(SRC)/pi_dartboard_serial.c | $(BIN)
	$(CC) $(CFLAGS_SERIAL) -o $(BIN)/$@ $< $(LDFLAGS)

# Versiones OpenMP Básicas
pi_needles_omp_basic: $(SRC)/pi_needles_omp_basic.c | $(BIN)
	$(CC) $(CFLAGS_OMP) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

pi_dartboard_omp_basic: $(SRC)/pi_dartboard_omp_basic.c | $(BIN)
	$(CC) $(CFLAGS_OMP) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

# Versiones OpenMP con diferentes scheduling policies
pi_needles_omp_static: $(SRC)/pi_needles_omp_static.c | $(BIN)
	$(CC) $(CFLAGS_OMP) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

pi_needles_omp_dynamic: $(SRC)/pi_needles_omp_dynamic.c | $(BIN)
	$(CC) $(CFLAGS_OMP) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

pi_needles_omp_guided: $(SRC)/pi_needles_omp_guided.c | $(BIN)
	$(CC) $(CFLAGS_OMP) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

pi_dartboard_omp_static: $(SRC)/pi_dartboard_omp_static.c | $(BIN)
	$(CC) $(CFLAGS_OMP) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

pi_dartboard_omp_dynamic: $(SRC)/pi_dartboard_omp_dynamic.c | $(BIN)
	$(CC) $(CFLAGS_OMP) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

pi_dartboard_omp_guided: $(SRC)/pi_dartboard_omp_guided.c | $(BIN)
	$(CC) $(CFLAGS_OMP) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

# Versiones Optimizadas (CPU + Memoria)
pi_needles_omp_optimized: $(SRC)/pi_needles_omp_optimized.c | $(BIN)
	$(CC) $(CFLAGS_OMP_OPT) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

pi_dartboard_omp_optimized: $(SRC)/pi_dartboard_omp_optimized.c | $(BIN)
	$(CC) $(CFLAGS_OMP_OPT) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

# Versiones para Profiling
pi_needles_omp_prof: $(SRC)/pi_needles_omp_basic.c | $(BIN)
	$(CC) $(CFLAGS_PROF) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

pi_dartboard_omp_prof: $(SRC)/pi_dartboard_omp_basic.c | $(BIN)
	$(CC) $(CFLAGS_PROF) -o $(BIN)/$@ $< $(LDFLAGS_OMP) $(LDFLAGS)

# Targets de utilidad
.PHONY: serial
serial: $(SERIAL_TARGETS)

.PHONY: omp_basic
omp_basic: $(OMP_BASIC_TARGETS)

.PHONY: omp_sched
omp_sched: $(OMP_SCHED_TARGETS)

.PHONY: omp_opt
omp_opt: $(OMP_OPT_TARGETS)

.PHONY: profiling
profiling: $(PROF_TARGETS)

# Crear directorios necesarios
$(BIN):
	mkdir -p $(BIN)

$(RESULTS):
	mkdir -p $(RESULTS)

# Limpiar
.PHONY: clean
clean:
	rm -f $(BIN)/*
	rm -f gmon.out
	rm -f $(RESULTS)/*.txt

.PHONY: clean-all
clean-all: clean
	rm -f $(RESULTS)/*.csv
	rm -f $(RESULTS)/*.dat

# Ayuda
.PHONY: help
help:
	@echo "=== HPCReto2 Makefile ==="
	@echo "Targets disponibles:"
	@echo "  make all         - Compila todas las versiones"
	@echo "  make serial      - Compila versiones seriales"
	@echo "  make omp_basic   - Compila versiones OpenMP básicas"
	@echo "  make omp_sched   - Compila versiones con diferentes scheduling"
	@echo "  make omp_opt     - Compila versiones optimizadas"
	@echo "  make profiling   - Compila versiones para profiling"
	@echo "  make clean       - Limpia binarios y archivos temporales"
	@echo "  make clean-all   - Limpia todo incluyendo resultados"
	@echo ""
	@echo "Flags de compilación:"
	@echo "  Serial:    $(CFLAGS_SERIAL)"
	@echo "  OMP:       $(CFLAGS_OMP)"
	@echo "  OMP_OPT:   $(CFLAGS_OMP_OPT)"
	@echo "  PROFILING: $(CFLAGS_PROF)"
