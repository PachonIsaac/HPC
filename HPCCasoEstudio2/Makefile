blocking_profile: $(SRC_DIR)/matrix_multiplication_blocking.c
	$(CC) -pg -o $(BIN_DIR)/matrix_multiplication_blocking $(SRC_DIR)/matrix_multiplication_blocking.c

blocking_seq_profile: $(SRC_DIR)/matrix_multiplication_blocking_seq.c
	$(CC) -pg -o $(BIN_DIR)/matrix_multiplication_blocking_seq $(SRC_DIR)/matrix_multiplication_blocking_seq.c
# This Makefile compila las diferentes versiones de multiplicación de matrices

CC = gcc
CFLAGS = -Wall -O2
CFLAGS_PROFILE = -pg

SRC_DIR = src
BIN_DIR = bin

all: secuencial optimizada paralela blocking secuencial_omp blocking_seq
blocking_seq: $(SRC_DIR)/matrix_multiplication_blocking_seq.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/matrix_multiplication_blocking_seq $(SRC_DIR)/matrix_multiplication_blocking_seq.c
secuencial_omp: $(SRC_DIR)/matrix_multiplication_seq_omp.c
	$(CC) $(CFLAGS) -fopenmp -o $(BIN_DIR)/matrix_multiplication_seq_omp $(SRC_DIR)/matrix_multiplication_seq_omp.c
blocking: $(SRC_DIR)/matrix_multiplication_blocking.c
	$(CC) $(CFLAGS) -fopenmp -o $(BIN_DIR)/matrix_multiplication_blocking $(SRC_DIR)/matrix_multiplication_blocking.c

secuencial: $(SRC_DIR)/matrix_multiplication.c
	$(CC) $(CFLAGS) -o $(BIN_DIR)/matrix_multiplication_sequential $(SRC_DIR)/matrix_multiplication.c

optimizada: $(SRC_DIR)/matrix_multiplication_optimized.c
	$(CC) $(CFLAGS) -fopenmp -o $(BIN_DIR)/matrix_multiplication_optimized $(SRC_DIR)/matrix_multiplication_optimized.c

paralela: $(SRC_DIR)/matrix_multiplication_parallel.c
	$(CC) $(CFLAGS) -fopenmp -o $(BIN_DIR)/matrix_multiplication_parallel $(SRC_DIR)/matrix_multiplication_parallel.c

	$(CC) $(CFLAGS_PROFILE) -o $(BIN_DIR)/matrix_multiplication_sequential $(SRC_DIR)/matrix_multiplication.c
	$(CC) $(CFLAGS_PROFILE) -fopenmp -o $(BIN_DIR)/matrix_multiplication_optimized $(SRC_DIR)/matrix_multiplication_optimized.c
	$(CC) $(CFLAGS_PROFILE) -fopenmp -o $(BIN_DIR)/matrix_multiplication_parallel $(SRC_DIR)/matrix_multiplication_parallel.c
	$(CC) $(CFLAGS_PROFILE) -fopenmp -o $(BIN_DIR)/matrix_multiplication_blocking $(SRC_DIR)/matrix_multiplication_blocking.c

clean:
	rm -f $(BIN_DIR)/*
