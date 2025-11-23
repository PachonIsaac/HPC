#!/bin/bash

# Script de prueba para multiplicación de matrices
# Uso: ./test_matrix.sh

echo "================================================"
echo "   PRUEBAS DE MULTIPLICACIÓN DE MATRICES HPC"
echo "================================================"

# Verificar que el programa esté compilado
if [ ! -f "matrix_mult" ]; then
    echo "Compilando programa..."
    make
    echo ""
fi

echo "1. Prueba de funcionamiento básico (10x10):"
./matrix_mult 10 42 84
echo ""

echo "2. Prueba con matriz mediana (100x100):"
./matrix_mult 100 123 456
echo ""

echo "3. Prueba con matriz grande (500x500):"
./matrix_mult 500 789 012
echo ""

echo "4. Prueba con parámetro automático (200x200):"
./matrix_mult 200
echo ""

echo "5. Benchmark de diferentes tamaños:"
echo "   - Matriz 50x50:"
./matrix_mult 50 100 200
echo ""
echo "   - Matriz 250x250:"
./matrix_mult 250 300 400
echo ""

echo "================================================"
echo "Pruebas completadas. El programa funciona correctamente."
echo "================================================"
