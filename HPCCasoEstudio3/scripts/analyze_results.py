#!/usr/bin/env python3
"""
analyze_results.py - Analiza resultados de benchmarks MPI
Calcula speedup, eficiencia, y genera visualizaciones
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
from pathlib import Path

# Configuración
RESULTS_FILE = "results/benchmarks.csv"
OUTPUT_DIR = "results"

def load_data():
    """Carga datos de benchmarks"""
    df = pd.read_csv(RESULTS_FILE)
    # Filtrar solo ejecuciones exitosas
    df = df[df['status'] == 'success'].copy()
    df['execution_time'] = pd.to_numeric(df['execution_time'])
    return df

def calculate_statistics(df):
    """Calcula promedios y desviación estándar por configuración"""
    # Agrupar por configuración y calcular estadísticas
    stats = df.groupby(['implementation', 'matrix_size', 'num_processes'])['execution_time'].agg([
        ('mean_time', 'mean'),
        ('std_time', 'std'),
        ('min_time', 'min'),
        ('max_time', 'max'),
        ('num_runs', 'count')
    ]).reset_index()
    
    return stats

def calculate_speedup(df):
    """Calcula speedup y eficiencia usando promedios"""
    results = []
    
    for impl in df['implementation'].unique():
        impl_df = df[df['implementation'] == impl]
        
        for size in df['matrix_size'].unique():
            size_df = impl_df[impl_df['matrix_size'] == size]
            
            # Baseline: sequential con menor número de procesos
            baseline_time = size_df[size_df['num_processes'] == size_df['num_processes'].min()]['mean_time'].values
            
            if len(baseline_time) == 0:
                continue
            baseline_time = baseline_time[0]
            
            for _, row in size_df.iterrows():
                procs = row['num_processes']
                time = row['mean_time']
                std = row['std_time']
                speedup = baseline_time / time
                efficiency = speedup / procs * 100
                
                results.append({
                    'implementation': impl,
                    'matrix_size': size,
                    'num_processes': procs,
                    'mean_time': time,
                    'std_time': std,
                    'min_time': row['min_time'],
                    'max_time': row['max_time'],
                    'num_runs': row['num_runs'],
                    'speedup': speedup,
                    'efficiency': efficiency,
                    'baseline_time': baseline_time
                })
    
    return pd.DataFrame(results)

def plot_speedup(df, output_dir):
    """Gráfica de speedup vs procesos con barras de error"""
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    
    sizes = sorted(df['matrix_size'].unique())
    
    for idx, size in enumerate(sizes):
        ax = axes[idx]
        size_df = df[df['matrix_size'] == size]
        
        for impl in df['implementation'].unique():
            impl_df = size_df[size_df['implementation'] == impl]
            if len(impl_df) > 0:
                # Calcular error bars del speedup
                speedup_std = impl_df['std_time'] / impl_df['baseline_time'] * impl_df['speedup']
                ax.errorbar(impl_df['num_processes'], impl_df['speedup'], 
                           yerr=speedup_std, marker='o', label=impl, 
                           linewidth=2, markersize=8, capsize=5)
        
        # Línea ideal
        procs = sorted(df['num_processes'].unique())
        ax.plot(procs, procs, 'k--', label='Ideal', alpha=0.5)
        
        ax.set_xlabel('Number of Processes', fontsize=12)
        ax.set_ylabel('Speedup', fontsize=12)
        ax.set_title(f'Matrix Size: {size}x{size}', fontsize=14, fontweight='bold')
        ax.legend()
        ax.grid(True, alpha=0.3)
        ax.set_xticks(procs)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/speedup_comparison.png", dpi=300, bbox_inches='tight')
    print(f"Saved: {output_dir}/speedup_comparison.png")
    plt.close()

def plot_efficiency(df, output_dir):
    """Gráfica de eficiencia"""
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))
    
    sizes = sorted(df['matrix_size'].unique())
    
    for idx, size in enumerate(sizes):
        ax = axes[idx]
        size_df = df[df['matrix_size'] == size]
        
        for impl in df['implementation'].unique():
            impl_df = size_df[size_df['implementation'] == impl]
            if len(impl_df) > 0:
                ax.plot(impl_df['num_processes'], impl_df['efficiency'], 
                       marker='s', label=impl, linewidth=2, markersize=8)
        
        ax.axhline(y=100, color='k', linestyle='--', alpha=0.5, label='Ideal (100%)')
        ax.set_xlabel('Number of Processes', fontsize=12)
        ax.set_ylabel('Efficiency (%)', fontsize=12)
        ax.set_title(f'Matrix Size: {size}x{size}', fontsize=14, fontweight='bold')
        ax.legend()
        ax.grid(True, alpha=0.3)
        ax.set_ylim(0, 120)
        procs = sorted(df['num_processes'].unique())
        ax.set_xticks(procs)
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/efficiency_comparison.png", dpi=300, bbox_inches='tight')
    print(f"Saved: {output_dir}/efficiency_comparison.png")
    plt.close()

def plot_execution_times(df, output_dir):
    """Gráfica de tiempos de ejecución con barras de error"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    # Crear tabla pivote
    implementations = sorted(df['implementation'].unique())
    x = np.arange(len(df['matrix_size'].unique()))
    width = 0.15
    
    for idx, impl in enumerate(implementations):
        impl_df = df[df['implementation'] == impl]
        
        for proc_idx, proc_count in enumerate(sorted(df['num_processes'].unique())):
            proc_df = impl_df[impl_df['num_processes'] == proc_count]
            if len(proc_df) > 0:
                times = []
                errors = []
                for size in sorted(df['matrix_size'].unique()):
                    size_data = proc_df[proc_df['matrix_size'] == size]
                    if len(size_data) > 0:
                        times.append(size_data['mean_time'].values[0])
                        errors.append(size_data['std_time'].values[0])
                    else:
                        times.append(0)
                        errors.append(0)
                
                offset = (idx * len(df['num_processes'].unique()) + proc_idx - 
                         len(implementations) * len(df['num_processes'].unique()) / 2) * width
                ax.bar(x + offset, times, width, yerr=errors,
                      label=f'{impl} ({proc_count}p)', alpha=0.8, capsize=3)
    
    ax.set_xlabel('Matrix Size', fontsize=12)
    ax.set_ylabel('Execution Time (seconds)', fontsize=12)
    ax.set_title('Execution Time Comparison (Mean ± StdDev)', fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels([f'{s}x{s}' for s in sorted(df['matrix_size'].unique())])
    ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left', fontsize=8)
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/execution_times.png", dpi=300, bbox_inches='tight')
    print(f"Saved: {output_dir}/execution_times.png")
    plt.close()

def plot_heatmap_speedup(df, output_dir):
    """Heatmap de speedup"""
    fig, axes = plt.subplots(2, 2, figsize=(14, 12))
    axes = axes.flatten()
    
    implementations = sorted(df['implementation'].unique())
    
    for idx, impl in enumerate(implementations):
        ax = axes[idx]
        impl_df = df[df['implementation'] == impl]
        
        # Crear pivote para heatmap
        pivot = impl_df.pivot_table(
            values='speedup',
            index='matrix_size',
            columns='num_processes',
            aggfunc='mean'
        )
        
        sns.heatmap(pivot, annot=True, fmt='.2f', cmap='RdYlGn', 
                   ax=ax, cbar_kws={'label': 'Speedup'},
                   vmin=0, vmax=6)
        ax.set_title(f'{impl.capitalize()} - Speedup', fontsize=12, fontweight='bold')
        ax.set_xlabel('Number of Processes')
        ax.set_ylabel('Matrix Size')
    
    plt.tight_layout()
    plt.savefig(f"{output_dir}/speedup_heatmap.png", dpi=300, bbox_inches='tight')
    print(f"Saved: {output_dir}/speedup_heatmap.png")
    plt.close()

def generate_summary_report(df, output_file):
    """Genera reporte de texto con estadísticas"""
    with open(output_file, 'w') as f:
        f.write("=" * 80 + "\n")
        f.write("MPI MATRIX MULTIPLICATION - BENCHMARK SUMMARY\n")
        f.write("=" * 80 + "\n\n")
        
        f.write("OVERALL STATISTICS\n")
        f.write("-" * 80 + "\n")
        f.write(f"Total configurations: {len(df)}\n")
        f.write(f"Runs per configuration: {int(df['num_runs'].iloc[0])}\n")
        f.write(f"Implementations tested: {', '.join(df['implementation'].unique())}\n")
        f.write(f"Matrix sizes: {', '.join(map(str, sorted(df['matrix_size'].unique())))}\n")
        f.write(f"Process counts: {', '.join(map(str, sorted(df['num_processes'].unique())))}\n\n")
        
        # Best results
        f.write("BEST SPEEDUP BY IMPLEMENTATION\n")
        f.write("-" * 80 + "\n")
        for impl in sorted(df['implementation'].unique()):
            impl_df = df[df['implementation'] == impl]
            best = impl_df.loc[impl_df['speedup'].idxmax()]
            f.write(f"{impl.capitalize():20s}: {best['speedup']:.2f}x ± {best['std_time']/best['baseline_time']*best['speedup']:.2f} ")
            f.write(f"(size={int(best['matrix_size'])}, procs={int(best['num_processes'])}, n={int(best['num_runs'])})\n")
        
        f.write("\n")
        f.write("BEST EFFICIENCY BY IMPLEMENTATION\n")
        f.write("-" * 80 + "\n")
        for impl in sorted(df['implementation'].unique()):
            impl_df = df[df['implementation'] == impl]
            best = impl_df.loc[impl_df['efficiency'].idxmax()]
            f.write(f"{impl.capitalize():20s}: {best['efficiency']:.1f}% ")
            f.write(f"(size={int(best['matrix_size'])}, procs={int(best['num_processes'])}, n={int(best['num_runs'])})\n")
        
        f.write("\n")
        f.write("DETAILED RESULTS BY MATRIX SIZE (Mean ± StdDev)\n")
        f.write("-" * 80 + "\n")
        
        for size in sorted(df['matrix_size'].unique()):
            f.write(f"\n{size}x{size} Matrix:\n")
            size_df = df[df['matrix_size'] == size]
            
            for impl in sorted(df['implementation'].unique()):
                impl_df = size_df[size_df['implementation'] == impl]
                if len(impl_df) > 0:
                    f.write(f"  {impl.capitalize()}\n")
                    for _, row in impl_df.sort_values('num_processes').iterrows():
                        f.write(f"    {int(row['num_processes'])} procs: ")
                        f.write(f"{row['mean_time']:8.4f}s ± {row['std_time']:6.4f}s  ")
                        f.write(f"Speedup: {row['speedup']:5.2f}x  ")
                        f.write(f"Efficiency: {row['efficiency']:5.1f}%  ")
                        f.write(f"(n={int(row['num_runs'])} runs)\n")
        
        f.write("\n")
        f.write("VARIABILITY ANALYSIS\n")
        f.write("-" * 80 + "\n")
        f.write(f"Average coefficient of variation: {(df['std_time']/df['mean_time']*100).mean():.2f}%\n")
        f.write(f"Max coefficient of variation: {(df['std_time']/df['mean_time']*100).max():.2f}%\n")
        f.write(f"Min coefficient of variation: {(df['std_time']/df['mean_time']*100).min():.2f}%\n")
        
        f.write("\n")
        f.write("=" * 80 + "\n")
    
    print(f"Saved: {output_file}")

def main():
    print("=== MPI Benchmark Analysis ===\n")
    
    # Verificar archivo de resultados
    if not Path(RESULTS_FILE).exists():
        print(f"Error: {RESULTS_FILE} not found")
        print("Run ./scripts/run_benchmarks.sh first")
        return
    
    # Cargar y procesar datos
    print("Loading data...")
    df_raw = load_data()
    
    if len(df_raw) == 0:
        print("No successful benchmark results found!")
        return
    
    print(f"Loaded {len(df_raw)} successful results")
    
    print("\nCalculating statistics (mean, std, min, max)...")
    df_stats = calculate_statistics(df_raw)
    
    print(f"Processed {len(df_stats)} unique configurations")
    
    print("\nCalculating speedup and efficiency...")
    df = calculate_speedup(df_stats)
    
    # Guardar resultados procesados
    processed_file = f"{OUTPUT_DIR}/processed_results.csv"
    df.to_csv(processed_file, index=False)
    print(f"Saved processed data: {processed_file}")
    
    # Generar visualizaciones
    print("\nGenerating plots...")
    plot_speedup(df, OUTPUT_DIR)
    plot_efficiency(df, OUTPUT_DIR)
    plot_execution_times(df, OUTPUT_DIR)
    plot_heatmap_speedup(df, OUTPUT_DIR)
    
    # Generar reporte
    print("\nGenerating summary report...")
    generate_summary_report(df, f"{OUTPUT_DIR}/summary_report.txt")
    
    print("\n=== Analysis Complete ===")
    print(f"\nResults saved in: {OUTPUT_DIR}/")
    print("  - processed_results.csv")
    print("  - summary_report.txt")
    print("  - speedup_comparison.png")
    print("  - efficiency_comparison.png")
    print("  - execution_times.png")
    print("  - speedup_heatmap.png")

if __name__ == "__main__":
    main()
