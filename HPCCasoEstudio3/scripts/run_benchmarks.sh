#!/bin/bash
# run_benchmarks.sh - Ejecuta benchmarks de todas las implementaciones MPI

# Configuración
MATRIX_SIZES=(600 1200 2400)  # Divisibles por 2, 4 y 6
PROCESS_COUNTS=(2 4 6)
IMPLEMENTATIONS=("sequential" "rowwise" "broadcast" "nonblocking")
RESULTS_FILE="./results/benchmarks.csv"
HOSTFILE="./hostfile"
NUM_RUNS=10  # Número de repeticiones por configuración

# Crear directorio de resultados
mkdir -p ./results

# Verificar hostfile
if [ ! -f "$HOSTFILE" ]; then
    echo "Error: hostfile not found at $HOSTFILE"
    echo "Create hostfile with:"
    echo "  localhost slots=2"
    echo "  worker1 slots=2"
    echo "  worker2 slots=2"
    exit 1
fi

echo "=== MPI Matrix Multiplication Benchmarks ==="
echo "Date: $(date)"
echo "Matrix sizes: ${MATRIX_SIZES[@]}"
echo "Process counts: ${PROCESS_COUNTS[@]}"
echo "Implementations: ${IMPLEMENTATIONS[@]}"
echo "Results file: $RESULTS_FILE"
echo ""

# Crear archivo CSV con headers
echo "implementation,matrix_size,num_processes,run_number,execution_time,status" > $RESULTS_FILE

# Función para ejecutar un benchmark
run_benchmark() {
    local impl=$1
    local size=$2
    local procs=$3
    local run=$4
    local binary="./bin/matrix_mpi_${impl}"
    
    echo -n "Run ${run}/${NUM_RUNS}: ${impl} | Size: ${size}x${size} | Procs: ${procs} ... "
    
    # Verificar que existe el binario
    if [ ! -f "$binary" ]; then
        echo "SKIP (binary not found)"
        echo "${impl},${size},${procs},${run},0.0,not_found" >> $RESULTS_FILE
        return
    fi
    
    # Ejecutar con timeout de 5 minutos
    output=$(timeout 300 mpirun --hostfile $HOSTFILE -np $procs $binary $size 2>&1)
    exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        # Extraer tiempo de ejecución del output
        time=$(echo "$output" | grep -i "total time" | awk '{print $3}')
        
        if [ -z "$time" ]; then
            time=$(echo "$output" | grep -i "computation time" | awk '{print $3}')
        fi
        
        if [ -z "$time" ]; then
            echo "FAILED (no time found)"
            echo "${impl},${size},${procs},${run},0.0,failed" >> $RESULTS_FILE
        else
            echo "OK (${time}s)"
            echo "${impl},${size},${procs},${run},${time},success" >> $RESULTS_FILE
        fi
    elif [ $exit_code -eq 124 ]; then
        echo "TIMEOUT"
        echo "${impl},${size},${procs},${run},0.0,timeout" >> $RESULTS_FILE
    else
        echo "ERROR (exit code: $exit_code)"
        echo "${impl},${size},${procs},${run},0.0,error" >> $RESULTS_FILE
    fi
}

# Ejecutar benchmarks
total_configs=$((${#IMPLEMENTATIONS[@]} * ${#MATRIX_SIZES[@]} * ${#PROCESS_COUNTS[@]}))
total_tests=$((total_configs * NUM_RUNS))
current_test=0

echo "Total configurations: $total_configs"
echo "Runs per configuration: $NUM_RUNS"
echo "Total tests to execute: $total_tests"
echo ""

for impl in "${IMPLEMENTATIONS[@]}"; do
    echo ""
    echo "=== Testing: $impl ==="
    for size in "${MATRIX_SIZES[@]}"; do
        for procs in "${PROCESS_COUNTS[@]}"; do
            echo "Configuration: ${impl} | ${size}x${size} | ${procs} processes"
            for run in $(seq 1 $NUM_RUNS); do
                current_test=$((current_test + 1))
                echo -n "  [$current_test/$total_tests] "
                run_benchmark $impl $size $procs $run
                sleep 1  # Pequeña pausa entre tests
            done
            echo ""
        done
    done
done

echo ""
echo "=== Benchmarks Complete ==="
echo "Results saved to: $RESULTS_FILE"
echo ""
echo "Summary:"
successful=$(grep ",success" $RESULTS_FILE | wc -l)
failed=$(grep -E ",failed|,error|,timeout" $RESULTS_FILE | wc -l)
echo "  Successful: $successful"
echo "  Failed: $failed"
echo "  Total: $total_tests"
echo ""
echo "To analyze results:"
echo "  python3 scripts/analyze_results.py"
