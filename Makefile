# Makefile para compilación de multiplicación de matrices
CC = gcc
CFLAGS = -O3 -Wall -Wextra -std=c99
PTHREAD_FLAGS = -pthread
RT_FLAGS = -lrt
TARGET = matrix_mult
TARGET_PTHREAD = matrix_mult_pthread
TARGET_PTHREAD_OPT = matrix_mult_pthread_opt
TARGET_PROCESSES = matrix_mult_processes
SOURCE = matrix_multiplication.c
SOURCE_PTHREAD = matrix_multiplication_pthread.c
SOURCE_PTHREAD_OPT = matrix_multiplication_pthread_optimized.c
SOURCE_PROCESSES = matrix_multiplication_processes.c

# Regla principal - versión secuencial
$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

# Regla para versión con pthreads
$(TARGET_PTHREAD): $(SOURCE_PTHREAD)
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) -o $(TARGET_PTHREAD) $(SOURCE_PTHREAD)

# Regla para versión pthread optimizada
$(TARGET_PTHREAD_OPT): $(SOURCE_PTHREAD_OPT)
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) -o $(TARGET_PTHREAD_OPT) $(SOURCE_PTHREAD_OPT)

# Regla para versión con procesos
$(TARGET_PROCESSES): $(SOURCE_PROCESSES)
	$(CC) $(CFLAGS) -o $(TARGET_PROCESSES) $(SOURCE_PROCESSES)

# Regla para compilación con optimizaciones adicionales
optimized: $(SOURCE)
	$(CC) -O3 -march=native -Wall -Wextra -std=c99 -o $(TARGET)_opt $(SOURCE)

# Regla para compilación optimizada con pthreads
optimized_pthread: $(SOURCE_PTHREAD)
	$(CC) -O3 -march=native -Wall -Wextra -std=c99 $(PTHREAD_FLAGS) -o $(TARGET_PTHREAD)_opt $(SOURCE_PTHREAD)

# Regla para compilación con información de debug
debug: $(SOURCE)
	$(CC) -g -Wall -Wextra -std=c99 -o $(TARGET)_debug $(SOURCE)

# Regla para debug con pthreads
debug_pthread: $(SOURCE_PTHREAD)
	$(CC) -g -Wall -Wextra -std=c99 $(PTHREAD_FLAGS) -o $(TARGET_PTHREAD)_debug $(SOURCE_PTHREAD)

# Regla para ejecutar pruebas rápidas
test: $(TARGET)
	@echo "=== Pruebas de funcionamiento ==="
	@echo "Matriz pequeña (10x10):"
	./$(TARGET) 10 123 456
	@echo ""
	@echo "Matriz mediana (100x100):"
	./$(TARGET) 100 789 012

# Regla para pruebas con pthreads
test_pthread: $(TARGET_PTHREAD)
	@echo "=== Pruebas con POSIX Threads ==="
	@echo "Matriz pequeña (50x50) con 2 hilos:"
	./$(TARGET_PTHREAD) 50 2 123 456
	@echo ""
	@echo "Matriz mediana (200x200) con 4 hilos:"
	./$(TARGET_PTHREAD) 200 4 789 012

# Regla para pruebas con procesos
test_processes: $(TARGET_PROCESSES)
	@echo "=== Pruebas con Procesos ==="
	@echo "Matriz pequeña (50x50) con 2 procesos:"
	./$(TARGET_PROCESSES) 50 2 123 456
	@echo ""
	@echo "Matriz mediana (200x200) con 4 procesos:"
	./$(TARGET_PROCESSES) 200 4 789 012

# Regla para benchmark con diferentes tamaños
benchmark: $(TARGET)
	@echo "=== Benchmark de rendimiento ==="
	@echo "Matriz 50x50:"
	./$(TARGET) 50
	@echo ""
	@echo "Matriz 100x100:"
	./$(TARGET) 100
	@echo ""
	@echo "Matriz 200x200:"
	./$(TARGET) 200
	@echo ""
	@echo "Matriz 500x500:"
	./$(TARGET) 500

# Benchmark comparativo entre todas las versiones
benchmark_all: $(TARGET) $(TARGET_PTHREAD_OPT) $(TARGET_PROCESSES)
	@echo "=== Benchmark Completo: Secuencial vs Hilos vs Procesos ==="
	@echo "--- Matriz 500x500 ---"
	@echo "Secuencial:"
	./$(TARGET) 500 123 456
	@echo ""
	@echo "Hilos (4 hilos):"
	./$(TARGET_PTHREAD_OPT) 500 4 123 456
	@echo ""
	@echo "Procesos (4 procesos):"
	./$(TARGET_PROCESSES) 500 4 123 456

# Limpiar archivos compilados
clean:
	rm -f $(TARGET) $(TARGET)_opt $(TARGET)_debug $(TARGET_PTHREAD) $(TARGET_PTHREAD)_opt $(TARGET_PTHREAD)_debug

# Compilar todo
all: $(TARGET) $(TARGET_PTHREAD) $(TARGET_PTHREAD_OPT) $(TARGET_PROCESSES)

# Ayuda
help:
	@echo "Opciones disponibles:"
	@echo "  make                    - Compilar versión secuencial"
	@echo "  make matrix_mult_pthread - Compilar versión con pthreads"
	@echo "  make all                - Compilar ambas versiones"
	@echo "  make optimized          - Compilar versión secuencial optimizada"
	@echo "  make optimized_pthread  - Compilar versión pthread optimizada"
	@echo "  make debug              - Compilar versión secuencial debug"
	@echo "  make debug_pthread      - Compilar versión pthread debug"
	@echo "  make test               - Ejecutar pruebas secuenciales"
	@echo "  make test_pthread       - Ejecutar pruebas con pthreads"
	@echo "  make benchmark          - Ejecutar benchmark secuencial"
	@echo "  make benchmark_compare  - Comparar secuencial vs paralelo"
	@echo "  make clean              - Limpiar archivos compilados"
	@echo "  make help               - Mostrar esta ayuda"

.PHONY: optimized optimized_pthread debug debug_pthread test test_pthread benchmark benchmark_compare clean all help
