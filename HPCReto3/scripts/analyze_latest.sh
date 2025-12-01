#!/bin/bash

# Script para analizar el último archivo de benchmarks
# Uso: ./scripts/analyze_latest.sh

set -e

ENVIRONMENT="${1:-local}"  # Argumento: local o cluster (default: local)
RESULTS_DIR="results/${ENVIRONMENT}"
ANALYSIS_DIR="${RESULTS_DIR}/analysis"

echo "=========================================="
echo "Analizador Automático de Últimos Resultados"
echo "Entorno: ${ENVIRONMENT}"
echo "=========================================="
echo ""

# Buscar el último archivo de benchmarks
LATEST_CSV=$(ls -t ${RESULTS_DIR}/benchmarks_*.csv 2>/dev/null | head -n 1)

if [ -z "$LATEST_CSV" ]; then
    echo "❌ No se encontraron archivos de benchmarks en ${RESULTS_DIR}/"
    echo ""
    echo "Ejecuta primero: ./scripts/run_benchmarks.sh"
    exit 1
fi

echo "📂 Último benchmark encontrado:"
echo "   $LATEST_CSV"
echo ""

# Mostrar info básica del CSV
NUM_LINES=$(($(wc -l < "$LATEST_CSV") - 1))
echo "📊 Datos:"
echo "   Total de tests: $NUM_LINES"
echo ""

# Verificar que el benchmark está completo
EXPECTED_MIN=200  # Número mínimo esperado de tests
if [ $NUM_LINES -lt $EXPECTED_MIN ]; then
    echo "⚠️  WARNING: El benchmark parece incompleto"
    echo "   Se esperaban al menos $EXPECTED_MIN tests, pero hay $NUM_LINES"
    echo ""
    read -p "¿Deseas continuar con el análisis de todos modos? (y/n) " -n 1 -r
    echo ""
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
    fi
fi

echo "=========================================="
echo "FASE 1: Análisis Estadístico Completo"
echo "=========================================="
echo ""

python3 scripts/analyze_results.py "$LATEST_CSV"

if [ $? -ne 0 ]; then
    echo ""
    echo "❌ Error en el análisis estadístico"
    exit 1
fi

echo ""
echo "=========================================="
echo "FASE 2: Gráficas de Presentación"
echo "=========================================="
echo ""

python3 scripts/generate_plots.py "$ANALYSIS_DIR"

if [ $? -ne 0 ]; then
    echo ""
    echo "❌ Error generando gráficas de presentación"
    exit 1
fi

echo ""
echo "=========================================="
echo "✅ ANÁLISIS COMPLETADO"
echo "=========================================="
echo ""
echo "📂 Todos los resultados en: $ANALYSIS_DIR/"
echo ""
echo "Resumen rápido:"
echo "  1. CSVs procesados:        $ANALYSIS_DIR/*.csv"
echo "  2. Gráficas básicas:       $ANALYSIS_DIR/*.png"
echo "  3. Gráficas presentación:  $ANALYSIS_DIR/plots_publication/*.png"
echo "  4. Reporte textual:        $ANALYSIS_DIR/REPORTE_ANALISIS.txt"
echo ""
echo "Para ver el reporte:"
echo "  cat $ANALYSIS_DIR/REPORTE_ANALISIS.txt"
echo ""
echo "Para abrir las gráficas:"
echo "  open $ANALYSIS_DIR/plots_publication/"
echo ""
