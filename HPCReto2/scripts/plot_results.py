#!/usr/bin/env python3
"""
Generación de Gráficas - HPCReto2
Crea visualizaciones de speedup, eficiencia y tiempos de ejecución
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import sys
import os

# Configurar estilo
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 10

def load_data(csv_file):
    """Carga datos del análisis de speedup"""
    if not os.path.exists(csv_file):
        print(f"Error: {csv_file} no encontrado")
        sys.exit(1)
    
    df = pd.read_csv(csv_file)
    print(f"✓ Cargados {len(df)} registros")
    return df

def plot_speedup_by_threads(df, algorithm, output_dir):
    """Gráfica de speedup vs número de threads"""
    algo_data = df[df['Algorithm'] == algorithm]
    
    fig, ax = plt.subplots(figsize=(12, 8))
    
    # Diferentes versiones
    versions = algo_data['Version'].unique()
    colors = plt.cm.tab10(np.linspace(0, 1, len(versions)))
    
    for version, color in zip(versions, colors):
        version_data = algo_data[algo_data['Version'] == version]
        
        # Agrupar por threads y promediar
        grouped = version_data.groupby('Threads')['Speedup'].mean().reset_index()
        
        ax.plot(grouped['Threads'], grouped['Speedup'], 
                marker='o', linewidth=2, markersize=8,
                label=version, color=color)
    
    # Línea ideal (speedup = threads)
    max_threads = algo_data['Threads'].max()
    threads_range = range(1, int(max_threads) + 1)
    ax.plot(threads_range, threads_range, 'k--', 
            linewidth=2, label='Ideal (lineal)', alpha=0.5)
    
    ax.set_xlabel('Número de Threads', fontsize=12, fontweight='bold')
    ax.set_ylabel('Speedup', fontsize=12, fontweight='bold')
    ax.set_title(f'Speedup vs Threads - {algorithm}', 
                 fontsize=14, fontweight='bold', pad=20)
    ax.legend(loc='best', fontsize=10)
    ax.grid(True, alpha=0.3)
    
    # Guardar
    filename = f'{algorithm.lower().replace(" ", "_")}_speedup_threads.png'
    filepath = os.path.join(output_dir, filename)
    plt.tight_layout()
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"✓ Guardada: {filepath}")

def plot_efficiency_by_threads(df, algorithm, output_dir):
    """Gráfica de eficiencia vs número de threads"""
    algo_data = df[df['Algorithm'] == algorithm]
    
    fig, ax = plt.subplots(figsize=(12, 8))
    
    versions = algo_data['Version'].unique()
    colors = plt.cm.tab10(np.linspace(0, 1, len(versions)))
    
    for version, color in zip(versions, colors):
        version_data = algo_data[algo_data['Version'] == version]
        grouped = version_data.groupby('Threads')['Efficiency_%'].mean().reset_index()
        
        ax.plot(grouped['Threads'], grouped['Efficiency_%'], 
                marker='s', linewidth=2, markersize=8,
                label=version, color=color)
    
    # Línea de 100% eficiencia
    ax.axhline(y=100, color='k', linestyle='--', linewidth=2, 
               label='100% Eficiencia', alpha=0.5)
    
    ax.set_xlabel('Número de Threads', fontsize=12, fontweight='bold')
    ax.set_ylabel('Eficiencia (%)', fontsize=12, fontweight='bold')
    ax.set_title(f'Eficiencia vs Threads - {algorithm}', 
                 fontsize=14, fontweight='bold', pad=20)
    ax.legend(loc='best', fontsize=10)
    ax.grid(True, alpha=0.3)
    ax.set_ylim(0, 110)
    
    filename = f'{algorithm.lower().replace(" ", "_")}_efficiency_threads.png'
    filepath = os.path.join(output_dir, filename)
    plt.tight_layout()
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"✓ Guardada: {filepath}")

def plot_time_comparison(df, algorithm, output_dir):
    """Gráfica comparativa de tiempos de ejecución"""
    algo_data = df[df['Algorithm'] == algorithm]
    
    # Tomar la configuración con 8 threads
    max_threads_data = algo_data[algo_data['Threads'] == 8]
    
    fig, ax = plt.subplots(figsize=(14, 8))
    
    versions = max_threads_data['Version'].unique()
    times = []
    labels = []
    
    for version in versions:
        version_data = max_threads_data[max_threads_data['Version'] == version]
        avg_time = version_data['Parallel_Time'].mean()
        times.append(avg_time)
        labels.append(version)
    
    # Crear gráfico de barras
    colors = plt.cm.viridis(np.linspace(0.2, 0.8, len(versions)))
    bars = ax.bar(range(len(labels)), times, color=colors, edgecolor='black', linewidth=1.5)
    
    # Añadir valores encima de las barras
    for i, (bar, time) in enumerate(zip(bars, times)):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height,
                f'{time:.4f}s',
                ha='center', va='bottom', fontsize=10, fontweight='bold')
    
    ax.set_xlabel('Versión', fontsize=12, fontweight='bold')
    ax.set_ylabel('Tiempo de Ejecución (segundos)', fontsize=12, fontweight='bold')
    ax.set_title(f'Comparación de Tiempos - {algorithm} ({max_threads_data["Threads"].values[0]} threads)', 
                 fontsize=14, fontweight='bold', pad=20)
    ax.set_xticks(range(len(labels)))
    ax.set_xticklabels(labels, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    
    filename = f'{algorithm.lower().replace(" ", "_")}_time_comparison.png'
    filepath = os.path.join(output_dir, filename)
    plt.tight_layout()
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"✓ Guardada: {filepath}")

def plot_scheduling_comparison(df, output_dir):
    """Compara diferentes políticas de scheduling"""
    scheduling_versions = ['OMP_Static', 'OMP_Dynamic', 'OMP_Guided']
    
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    
    for idx, algorithm in enumerate(df['Algorithm'].unique()):
        algo_data = df[df['Algorithm'] == algorithm]
        sched_data = algo_data[algo_data['Version'].isin(scheduling_versions)]
        
        if len(sched_data) == 0:
            continue
        
        ax = axes[idx]
        
        for version in scheduling_versions:
            version_data = sched_data[sched_data['Version'] == version]
            if len(version_data) > 0:
                grouped = version_data.groupby('Threads')['Speedup'].mean().reset_index()
                ax.plot(grouped['Threads'], grouped['Speedup'], 
                       marker='o', linewidth=2, markersize=8, label=version)
        
        ax.set_xlabel('Número de Threads', fontsize=11, fontweight='bold')
        ax.set_ylabel('Speedup', fontsize=11, fontweight='bold')
        ax.set_title(f'{algorithm}', fontsize=12, fontweight='bold')
        ax.legend(loc='best')
        ax.grid(True, alpha=0.3)
    
    fig.suptitle('Comparación de Políticas de Scheduling', 
                 fontsize=14, fontweight='bold', y=1.02)
    
    filename = 'scheduling_comparison.png'
    filepath = os.path.join(output_dir, filename)
    plt.tight_layout()
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"✓ Guardada: {filepath}")

def plot_scalability_analysis(df, output_dir):
    """Análisis de escalabilidad (Strong Scaling)"""
    fig, axes = plt.subplots(1, 2, figsize=(16, 6))
    
    for idx, algorithm in enumerate(df['Algorithm'].unique()):
        algo_data = df[df['Algorithm'] == algorithm]
        
        # Usar versión OMP_Basic para análisis de escalabilidad
        basic_data = algo_data[algo_data['Version'] == 'OMP_Basic']
        
        if len(basic_data) == 0:
            continue
        
        ax = axes[idx]
        
        iterations_list = sorted(basic_data['Iterations'].unique())
        colors = plt.cm.rainbow(np.linspace(0, 1, len(iterations_list)))
        
        for iterations, color in zip(iterations_list, colors):
            iter_data = basic_data[basic_data['Iterations'] == iterations]
            grouped = iter_data.groupby('Threads')['Speedup'].mean().reset_index()
            
            ax.plot(grouped['Threads'], grouped['Speedup'], 
                   marker='o', linewidth=2, markersize=8,
                   label=f'{iterations:,}', color=color)
        
        # Línea ideal
        max_threads = basic_data['Threads'].max()
        threads_range = range(1, int(max_threads) + 1)
        ax.plot(threads_range, threads_range, 'k--', 
               linewidth=2, label='Ideal', alpha=0.5)
        
        ax.set_xlabel('Número de Threads', fontsize=11, fontweight='bold')
        ax.set_ylabel('Speedup', fontsize=11, fontweight='bold')
        ax.set_title(f'{algorithm}', fontsize=12, fontweight='bold')
        ax.legend(title='Iteraciones', loc='best', fontsize=9)
        ax.grid(True, alpha=0.3)
    
    fig.suptitle('Análisis de Escalabilidad (Strong Scaling)', 
                 fontsize=14, fontweight='bold', y=1.02)
    
    filename = 'scalability_analysis.png'
    filepath = os.path.join(output_dir, filename)
    plt.tight_layout()
    plt.savefig(filepath, dpi=300, bbox_inches='tight')
    plt.close()
    
    print(f"✓ Guardada: {filepath}")

def main():
    """Función principal"""
    if len(sys.argv) < 2:
        print("Uso: python3 plot_results.py <speedup_analysis.csv>")
        print("Ejemplo: python3 plot_results.py ../results/speedup_analysis.csv")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    output_dir = os.path.join(os.path.dirname(csv_file), 'plots')
    os.makedirs(output_dir, exist_ok=True)
    
    print("="*80)
    print("GENERACIÓN DE GRÁFICAS - HPCReto2")
    print("="*80)
    
    # Cargar datos
    print(f"\n📂 Cargando {csv_file}...")
    df = load_data(csv_file)
    
    print("\n📊 Generando gráficas...")
    
    # Gráficas por algoritmo
    for algorithm in df['Algorithm'].unique():
        print(f"\n  → {algorithm}")
        plot_speedup_by_threads(df, algorithm, output_dir)
        plot_efficiency_by_threads(df, algorithm, output_dir)
        plot_time_comparison(df, algorithm, output_dir)
    
    # Gráficas comparativas
    print("\n  → Comparaciones")
    plot_scheduling_comparison(df, output_dir)
    plot_scalability_analysis(df, output_dir)
    
    print("\n" + "="*80)
    print(f"✓ Todas las gráficas guardadas en: {output_dir}")
    print("="*80)

if __name__ == "__main__":
    main()
