#!/usr/bin/env python3
"""
Análisis de Benchmarks - HPCReto2
Calcula speedup, eficiencia y genera tablas de resultados
"""

import pandas as pd
import numpy as np
import sys
import os

def load_benchmark_data(csv_file):
    """Carga datos del CSV de benchmarks"""
    if not os.path.exists(csv_file):
        print(f"Error: {csv_file} no encontrado")
        sys.exit(1)
    
    df = pd.read_csv(csv_file)
    print(f"✓ Cargados {len(df)} registros de {csv_file}")
    return df

def calculate_statistics(df):
    """Calcula promedios y desviación estándar por configuración"""
    # Agrupar por configuración
    grouped = df.groupby(['Algorithm', 'Version', 'Iterations', 'Threads']).agg({
        'Time_Seconds': ['mean', 'std', 'min', 'max'],
        'PI_Estimate': ['mean', 'std']
    }).reset_index()
    
    # Aplanar nombres de columnas
    grouped.columns = ['Algorithm', 'Version', 'Iterations', 'Threads',
                       'Avg_Time', 'Std_Time', 'Min_Time', 'Max_Time',
                       'Avg_PI', 'Std_PI']
    
    return grouped

def calculate_speedup(stats_df):
    """Calcula speedup y eficiencia respecto a la versión serial"""
    results = []
    
    for algorithm in stats_df['Algorithm'].unique():
        algo_data = stats_df[stats_df['Algorithm'] == algorithm]
        
        for iterations in algo_data['Iterations'].unique():
            # Obtener tiempo serial (baseline)
            serial_data = algo_data[
                (algo_data['Iterations'] == iterations) & 
                (algo_data['Version'] == 'Serial')
            ]
            
            if len(serial_data) == 0:
                print(f"Warning: No serial data for {algorithm} with {iterations} iterations")
                continue
            
            serial_time = serial_data['Avg_Time'].values[0]
            
            # Calcular speedup para cada versión paralela
            parallel_data = algo_data[
                (algo_data['Iterations'] == iterations) & 
                (algo_data['Version'] != 'Serial')
            ]
            
            for _, row in parallel_data.iterrows():
                speedup = serial_time / row['Avg_Time']
                efficiency = (speedup / row['Threads']) * 100
                
                results.append({
                    'Algorithm': algorithm,
                    'Version': row['Version'],
                    'Iterations': iterations,
                    'Threads': row['Threads'],
                    'Serial_Time': serial_time,
                    'Parallel_Time': row['Avg_Time'],
                    'Speedup': speedup,
                    'Efficiency_%': efficiency,
                    'Avg_PI': row['Avg_PI'],
                    'Std_PI': row['Std_PI']
                })
    
    return pd.DataFrame(results)

def print_summary(speedup_df):
    """Imprime resumen de resultados"""
    print("\n" + "="*80)
    print("RESUMEN DE SPEEDUP Y EFICIENCIA")
    print("="*80)
    
    for algorithm in speedup_df['Algorithm'].unique():
        print(f"\n{'='*80}")
        print(f"  {algorithm}")
        print(f"{'='*80}")
        
        algo_data = speedup_df[speedup_df['Algorithm'] == algorithm]
        
        for iterations in sorted(algo_data['Iterations'].unique()):
            iter_data = algo_data[algo_data['Iterations'] == iterations]
            
            print(f"\n  Iteraciones: {iterations:,}")
            print(f"  {'Version':<25} {'Threads':<8} {'Time(s)':<10} {'Speedup':<10} {'Efficiency'}")
            print(f"  {'-'*70}")
            
            for _, row in iter_data.iterrows():
                print(f"  {row['Version']:<25} {row['Threads']:<8} "
                      f"{row['Parallel_Time']:>8.4f}   {row['Speedup']:>8.2f}x   "
                      f"{row['Efficiency_%']:>7.1f}%")

def find_best_configurations(speedup_df):
    """Encuentra las mejores configuraciones"""
    print("\n" + "="*80)
    print("MEJORES CONFIGURACIONES")
    print("="*80)
    
    for algorithm in speedup_df['Algorithm'].unique():
        algo_data = speedup_df[speedup_df['Algorithm'] == algorithm]
        
        # Mejor speedup
        best_speedup = algo_data.loc[algo_data['Speedup'].idxmax()]
        print(f"\n{algorithm} - Mejor Speedup:")
        print(f"  Version: {best_speedup['Version']}")
        print(f"  Threads: {best_speedup['Threads']}")
        print(f"  Speedup: {best_speedup['Speedup']:.2f}x")
        print(f"  Eficiencia: {best_speedup['Efficiency_%']:.1f}%")
        
        # Mejor eficiencia
        best_eff = algo_data.loc[algo_data['Efficiency_%'].idxmax()]
        print(f"\n{algorithm} - Mejor Eficiencia:")
        print(f"  Version: {best_eff['Version']}")
        print(f"  Threads: {best_eff['Threads']}")
        print(f"  Speedup: {best_eff['Speedup']:.2f}x")
        print(f"  Eficiencia: {best_eff['Efficiency_%']:.1f}%")

def save_results(stats_df, speedup_df, output_dir):
    """Guarda resultados en archivos CSV"""
    os.makedirs(output_dir, exist_ok=True)
    
    stats_file = os.path.join(output_dir, 'statistics.csv')
    speedup_file = os.path.join(output_dir, 'speedup_analysis.csv')
    
    stats_df.to_csv(stats_file, index=False)
    speedup_df.to_csv(speedup_file, index=False)
    
    print(f"\n✓ Estadísticas guardadas en: {stats_file}")
    print(f"✓ Análisis de speedup guardado en: {speedup_file}")

def main():
    """Función principal"""
    # Verificar argumentos
    if len(sys.argv) < 2:
        print("Uso: python3 analyze_benchmarks.py <benchmarks.csv>")
        print("Ejemplo: python3 analyze_benchmarks.py ../results/benchmarks.csv")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    print("="*80)
    print("ANÁLISIS DE BENCHMARKS - HPCReto2")
    print("="*80)
    
    # Cargar datos
    df = load_benchmark_data(csv_file)
    
    # Calcular estadísticas
    print("\n📊 Calculando estadísticas...")
    stats_df = calculate_statistics(df)
    
    # Calcular speedup
    print("📈 Calculando speedup y eficiencia...")
    speedup_df = calculate_speedup(stats_df)
    
    # Imprimir resumen
    print_summary(speedup_df)
    
    # Encontrar mejores configuraciones
    find_best_configurations(speedup_df)
    
    # Guardar resultados
    output_dir = os.path.dirname(csv_file)
    save_results(stats_df, speedup_df, output_dir)
    
    print("\n" + "="*80)
    print("✓ Análisis completado")
    print("="*80)

if __name__ == "__main__":
    main()
