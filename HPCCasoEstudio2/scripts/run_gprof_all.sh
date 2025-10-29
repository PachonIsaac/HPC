#!/bin/bash

BIN_DIR="bin"
PROFILE_DIR="gprof_reports"
MATRIX_SIZE=400  # Puedes cambiar el tamaño aquí
SEED_A=12345
SEED_B=54321

VERSIONS=(
    "matrix_multiplication_sequential"
    "matrix_multiplication_optimized"
    "matrix_multiplication_parallel"
    "matrix_multiplication_blocking"
    "matrix_multiplication_seq_omp"
    "matrix_multiplication_blocking_seq"
)

mkdir -p "$PROFILE_DIR"

for version in "${VERSIONS[@]}"; do
    echo "Ejecutando $version..."
    ./$BIN_DIR/$version $MATRIX_SIZE $SEED_A $SEED_B
    gprof ./$BIN_DIR/$version gmon.out > "$PROFILE_DIR/${version}_gprof.txt"
    rm -f gmon.out
    echo "Reporte guardado en $PROFILE_DIR/${version}_gprof.txt"
done

echo "Perfilado completado. Revisa los reportes en $PROFILE_DIR/"
