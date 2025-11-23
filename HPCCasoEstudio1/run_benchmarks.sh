#!/usr/bin/env bash
# Benchmark automático de matrix_mult_all en modo CSV
# Genera archivo: benchmarks.csv

set -euo pipefail

BIN=./matrix_mult_all
OUT=benchmarks.csv
REPS=10
SIZES=(10 100 200 400 800 1600 3200)
WORKERS=(2 4 8 12 16)
SEED_A=123
SEED_B=456

if [[ ! -x $BIN ]]; then
  echo "Error: no se encuentra $BIN ejecutable. Compila primero." >&2
  exit 1
fi

echo "Creando encabezado CSV en $OUT" >&2
$BIN --csv-header > "$OUT"

run_id=1
for n in "${SIZES[@]}"; do
  for w in "${WORKERS[@]}"; do
    echo "== Tamaño $n | Workers $w ==" >&2
    for ((r=1; r<=REPS; r++)); do
      # Ejecutar y añadir línea
      $BIN "$n" "$w" $SEED_A $SEED_B --csv --run=$run_id >> "$OUT"
      run_id=$((run_id+1))
    done
  done
done

echo "Benchmark completado. Archivo: $OUT" >&2
