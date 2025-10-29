#!/bin/bash
# Script para verificar que todas las versiones de multiplicación de matrices generan el mismo resultado

BIN_DIR="bin"
MATRIX_SIZES=(100 200 400 800 1600)
REPEATS=3
THREADS=8
SEED_A=12345
SEED_B=54321

# Función para obtener la suma de verificación de una versión
get_sum() {
    local version=$1
    local size=$2
    local threads=$3
    if [ "$version" == "parallel" ]; then
        export OMP_NUM_THREADS=$threads
    fi
    ./"$BIN_DIR"/matrix_multiplication_$version $size $SEED_A $SEED_B | grep "Suma de verificación" | awk -F":" '{print $2}' | tr -d ' '
}

for size in "${MATRIX_SIZES[@]}"; do
    echo "Verificando tamaño de matriz $size..."
    # Ejecutar secuencial y guardar suma
    sum_ref=$(./$BIN_DIR/matrix_multiplication_sequential $size $SEED_A $SEED_B | grep "Suma de verificación" | awk -F":" '{print $2}' | tr -d ' ')
    echo "Secuencial: $sum_ref"
    for version in optimized parallel blocking seq_omp blocking_seq; do
        for ((i=1; i<=REPEATS; i++)); do
            sum=$(get_sum $version $size $THREADS)
            if [ "$sum" == "$sum_ref" ]; then
                echo "[OK] $version ($size) repetición $i: suma $sum"
            else
                echo "[ERROR] $version ($size) repetición $i: suma $sum (esperado $sum_ref)"
            fi
        done
    done
    echo "---"
done

echo "Verificación completada."
