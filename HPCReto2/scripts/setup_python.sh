#!/bin/bash
# Setup script - Instala dependencias de Python necesarias

echo "======================================"
echo " Setup HPCReto2 - Python Dependencies"
echo "======================================"

echo -e "\nVerificando Python..."
python3 --version

echo -e "\nInstalando dependencias..."
pip3 install --user pandas numpy matplotlib seaborn

echo -e "\n✓ Dependencias instaladas"
echo -e "\nNow you can run:"
echo "  python3 analyze_benchmarks.py ../results/benchmarks.csv"
echo "  python3 plot_results.py ../results/speedup_analysis.csv"
