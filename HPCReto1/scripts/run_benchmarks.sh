#!/bin/bash
# Script automático para ejecutar benchmarks y guardar resultados en benchmarks.csv

ITERATIONS=(10000000 40000000 80000000 100000000)
THREADS_PROCS=(2 4 8 12)
REPEAT=5
BIN_DIR="../bin"
CSV="../benchmarksReto1.csv"

# Limpiar archivo CSV y escribir encabezado
> "$CSV"
echo "Algoritmo,Tipo,Hilos/Procesos,Iteraciones,Tiempo(s)" >> "$CSV"

run_test() {
    local cmd="$1"
    local tipo="$2"
    local alg="$3"
    local iter="$4"
    local par="$5"
    for r in $(seq 1 $REPEAT); do
        TIME=$($cmd $iter $par | grep "Tiempo de ejecución" | awk -F ': ' '{print $2}' | awk '{print $1}')
        if [[ -z "$TIME" ]]; then
            TIME="N/A"
        fi
        echo "$alg,$tipo,$par,$iter,$TIME" >> "$CSV"
    done
}

# Algoritmos seriales
for iter in "${ITERATIONS[@]}"; do
    for r in $(seq 1 $REPEAT); do
    TIME=$($BIN_DIR/pi_needles_serial $iter | grep "Tiempo de ejecución" | awk -F ': ' '{print $2}' | awk '{print $1}')
    if [[ -z "$TIME" ]]; then TIME="N/A"; fi
    echo "Needles,Serial,1,$iter,$TIME" >> "$CSV"
    TIME=$($BIN_DIR/pi_needles_serial_opt $iter | grep "Tiempo de ejecución" | awk -F ': ' '{print $2}' | awk '{print $1}')
    if [[ -z "$TIME" ]]; then TIME="N/A"; fi
    echo "Needles,Serial-Opt,1,$iter,$TIME" >> "$CSV"
    TIME=$($BIN_DIR/pi_dartboard_serial $iter | grep "Tiempo de ejecución" | awk -F ': ' '{print $2}' | awk '{print $1}')
    if [[ -z "$TIME" ]]; then TIME="N/A"; fi
    echo "Dartboard,Serial,1,$iter,$TIME" >> "$CSV"
    TIME=$($BIN_DIR/pi_dartboard_serial_opt $iter | grep "Tiempo de ejecución" | awk -F ': ' '{print $2}' | awk '{print $1}')
    if [[ -z "$TIME" ]]; then TIME="N/A"; fi
    echo "Dartboard,Serial-Opt,1,$iter,$TIME" >> "$CSV"
    done
    # Algoritmos con threads y procesos
    for par in "${THREADS_PROCS[@]}"; do
        run_test "$BIN_DIR/pi_needles_thread" "Threads" "Needles" "$iter" "$par"
        run_test "$BIN_DIR/pi_needles_fork" "Fork" "Needles" "$iter" "$par"
        run_test "$BIN_DIR/pi_dartboard_thread" "Threads" "Dartboard" "$iter" "$par"
        run_test "$BIN_DIR/pi_dartboard_fork" "Fork" "Dartboard" "$iter" "$par"
    done

done

echo "Benchmark finalizado. Resultados en $CSV"
