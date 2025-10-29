#!/bin/bash

BIN_DIR="bin"
RESULTS_FILE="results.csv"
MATRIX_SIZES=(100 200 400 800 1600 3200)
REPEATS=5
THREADS=8


# Encabezado del archivo CSV
echo "version,tamaño_matriz,tiempo_wall,speedup" > $RESULTS_FILE


run_version() {
    local version=$1
    local size=$2
    local repeats=$3
    local threads=$4

    total_wall_time=0

    for ((i=1; i<=repeats; i++)); do
        if [ "$version" == "parallel" ]; then
            export OMP_NUM_THREADS=$threads
        fi

        wall_time=$(./$BIN_DIR/matrix_multiplication_$version $size | grep "Tiempo real" | awk '{print $5}')
        total_wall_time=$(echo "$total_wall_time + $wall_time" | bc -l)
    done

    avg_wall_time=$(echo "$total_wall_time / $repeats" | bc -l)
    echo "$avg_wall_time"
}


for size in "${MATRIX_SIZES[@]}"; do
    echo "Ejecutando secuencial para tamaño $size..."
    seq_total=0
    for ((i=1; i<=REPEATS; i++)); do
        wall_time=$(./$BIN_DIR/matrix_multiplication_sequential $size | grep "Tiempo real" | awk '{print $5}')
        seq_total=$(echo "$seq_total + $wall_time" | bc -l)
        echo "secuencial,$size,$wall_time,1" >> $RESULTS_FILE
    done
    seq_avg=$(echo "$seq_total / $REPEATS" | bc -l)

    for version in "optimized" "parallel" "blocking" "seq_omp" "blocking_seq"; do
        echo "Ejecutando $version para tamaño $size..."
        for ((i=1; i<=REPEATS; i++)); do
            if [ "$version" == "parallel" ]; then
                export OMP_NUM_THREADS=$THREADS
            fi
            wall_time=$(./$BIN_DIR/matrix_multiplication_$version $size | grep "Tiempo real" | awk '{print $5}')
            speedup=$(echo "$seq_avg / $wall_time" | bc -l)
            echo "$version,$size,$wall_time,$speedup" >> $RESULTS_FILE
        done
    done
done

echo "Pruebas completadas. Resultados guardados en $RESULTS_FILE."