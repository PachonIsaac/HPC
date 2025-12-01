#!/usr/bin/env python3
"""
Script para generar gráficas de presentación de alta calidad
Crea visualizaciones listas para informe técnico
Autor: Isaac Pachon
Fecha: 2025-11-29
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import sys
import os

# Configuración de estilo profesional
sns.set_style("whitegrid")
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['font.size'] = 11
plt.rcParams['axes.labelsize'] = 12
plt.rcParams['axes.titlesize'] = 14
plt.rcParams['legend.fontsize'] = 10
plt.rcParams['figure.dpi'] = 100
plt.rcParams['savefig.dpi'] = 300
plt.rcParams['savefig.bbox'] = 'tight'

# Colores consistentes
COLORS = {
    'blocking': '#E74C3C',      # Rojo
    'nonblocking': '#3498DB',   # Azul
    'serial': '#95A5A6',        # Gris
    'ideal': '#2ECC71'          # Verde
}

def load_speedup_data(analysis_dir):
    """Carga datos de speedup/efficiency"""
    speedup_file = f"{analysis_dir}/speedup_efficiency.csv"
    if not os.path.exists(speedup_file):
        print(f"❌ Error: {speedup_file} no encontrado")
        print("Ejecuta primero: python3 scripts/analyze_results.py <benchmarks.csv>")
        sys.exit(1)
    
    df = pd.read_csv(speedup_file)
    print(f"✓ Datos cargados: {len(df)} configuraciones")
    return df

def create_publication_speedup_plot(df, output_file):
    """Gráfica de speedup estilo publicación científica"""
    print("📊 Creando gráfica de speedup para publicación...")
    
    # Filtrar N más grande para strong scaling
    N_max = df['N'].max()
    df_strong = df[df['N'] == N_max].copy()
    
    fig, ax = plt.subplots(figsize=(8, 6))
    
    # Plot para cada versión MPI
    for version, color in [('mpi_blocking', COLORS['blocking']), 
                           ('mpi_nonblocking', COLORS['nonblocking'])]:
        df_v = df_strong[df_strong['version'] == version].sort_values('P')
        
        if len(df_v) == 0:
            continue
        
        label = 'MPI Blocking' if 'blocking' in version else 'MPI Non-Blocking'
        
        ax.plot(df_v['P'], df_v['speedup'], 
               marker='o', markersize=8, linewidth=2.5,
               color=color, label=label, alpha=0.9)
        
        # Añadir valores numéricos
        for _, row in df_v.iterrows():
            ax.annotate(f"{row['speedup']:.2f}", 
                       xy=(row['P'], row['speedup']),
                       xytext=(0, 8), textcoords='offset points',
                       ha='center', fontsize=9, color=color)
    
    # Línea de speedup ideal
    P_vals = sorted(df_strong['P'].unique())
    ax.plot(P_vals, P_vals, 'k--', linewidth=2, 
           label='Speedup Ideal', alpha=0.6)
    
    # Configuración de ejes
    ax.set_xlabel('Número de Procesos (P)', fontweight='bold')
    ax.set_ylabel('Speedup S(P)', fontweight='bold')
    ax.set_title(f'Strong Scaling Analysis\n(N = {N_max:,} cells, T = 100 timesteps)', 
                fontweight='bold', pad=15)
    
    # Personalización
    ax.legend(frameon=True, shadow=True, loc='upper left')
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.set_xticks(P_vals)
    
    # Limites
    ax.set_xlim(0.5, max(P_vals) + 0.5)
    ax.set_ylim(0, max(P_vals) * 1.15)
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def create_efficiency_comparison_plot(df, output_file):
    """Gráfica comparativa de eficiencia"""
    print("📊 Creando gráfica de eficiencia...")
    
    N_max = df['N'].max()
    df_strong = df[df['N'] == N_max].copy()
    
    fig, ax = plt.subplots(figsize=(8, 6))
    
    # Barras agrupadas
    versions = ['mpi_blocking', 'mpi_nonblocking']
    labels = ['Blocking', 'Non-Blocking']
    
    P_vals = sorted(df_strong['P'].unique())
    x = np.arange(len(P_vals))
    width = 0.35
    
    for idx, (version, label) in enumerate(zip(versions, labels)):
        df_v = df_strong[df_strong['version'] == version].sort_values('P')
        
        if len(df_v) == 0:
            continue
        
        color = COLORS['blocking'] if 'blocking' in version else COLORS['nonblocking']
        
        bars = ax.bar(x + idx * width, df_v['efficiency'], width,
                     label=label, color=color, alpha=0.8, edgecolor='black')
        
        # Añadir valores sobre barras
        for bar, val in zip(bars, df_v['efficiency']):
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height,
                   f'{val:.1f}%', ha='center', va='bottom', fontsize=9)
    
    # Línea de 100% eficiencia
    ax.axhline(y=100, color='green', linestyle='--', linewidth=2, 
              label='Eficiencia Ideal (100%)', alpha=0.6)
    
    # Configuración
    ax.set_xlabel('Número de Procesos (P)', fontweight='bold')
    ax.set_ylabel('Eficiencia E(P) (%)', fontweight='bold')
    ax.set_title(f'Parallel Efficiency Comparison\n(N = {N_max:,} cells)', 
                fontweight='bold', pad=15)
    ax.set_xticks(x + width / 2)
    ax.set_xticklabels(P_vals)
    ax.legend(frameon=True, shadow=True)
    ax.grid(True, alpha=0.3, axis='y', linestyle='--')
    ax.set_ylim(0, 115)
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def create_performance_improvement_plot(df, output_file):
    """Gráfica de mejora porcentual Non-Blocking sobre Blocking"""
    print("📊 Creando gráfica de mejora de performance...")
    
    df_blocking = df[df['version'] == 'mpi_blocking'].copy()
    df_nonblocking = df[df['version'] == 'mpi_nonblocking'].copy()
    
    # Merge
    merged = pd.merge(df_blocking, df_nonblocking,
                     on=['N', 'T', 'density', 'P'],
                     suffixes=('_block', '_nonblock'))
    
    if len(merged) == 0:
        print("  ⚠️  No hay datos pareados para comparar")
        return
    
    # Calcular mejora
    merged['improvement'] = ((merged['time_mean_block'] - merged['time_mean_nonblock']) 
                            / merged['time_mean_block'] * 100)
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    # 1. Mejora por número de procesos
    improvement_by_p = merged.groupby('P').agg({
        'improvement': ['mean', 'std', 'min', 'max']
    }).reset_index()
    improvement_by_p.columns = ['P', 'mean', 'std', 'min', 'max']
    
    ax1.bar(improvement_by_p['P'], improvement_by_p['mean'],
           color=COLORS['nonblocking'], alpha=0.7, edgecolor='black',
           yerr=improvement_by_p['std'], capsize=5, label='Promedio ± Std')
    
    # Añadir valores
    for _, row in improvement_by_p.iterrows():
        ax1.text(row['P'], row['mean'], f"{row['mean']:.1f}%",
                ha='center', va='bottom', fontsize=10, fontweight='bold')
    
    ax1.axhline(y=0, color='black', linestyle='-', linewidth=1)
    ax1.set_xlabel('Número de Procesos (P)', fontweight='bold')
    ax1.set_ylabel('Mejora de Performance (%)', fontweight='bold')
    ax1.set_title('Non-Blocking vs Blocking\nMejora por Procesos', 
                 fontweight='bold', pad=15)
    ax1.grid(True, alpha=0.3, axis='y', linestyle='--')
    ax1.set_xticks(improvement_by_p['P'])
    
    # 2. Mejora por tamaño del problema
    improvement_by_n = merged.groupby('N').agg({
        'improvement': ['mean', 'std']
    }).reset_index()
    improvement_by_n.columns = ['N', 'mean', 'std']
    
    ax2.plot(improvement_by_n['N'], improvement_by_n['mean'],
            marker='o', markersize=10, linewidth=2.5,
            color=COLORS['nonblocking'], label='Mejora Promedio')
    
    ax2.fill_between(improvement_by_n['N'],
                     improvement_by_n['mean'] - improvement_by_n['std'],
                     improvement_by_n['mean'] + improvement_by_n['std'],
                     color=COLORS['nonblocking'], alpha=0.2,
                     label='± Desviación Estándar')
    
    ax2.axhline(y=0, color='black', linestyle='-', linewidth=1)
    ax2.set_xlabel('Tamaño del Problema (N)', fontweight='bold')
    ax2.set_ylabel('Mejora de Performance (%)', fontweight='bold')
    ax2.set_title('Non-Blocking vs Blocking\nMejora por Tamaño', 
                 fontweight='bold', pad=15)
    ax2.grid(True, alpha=0.3, linestyle='--')
    ax2.legend(frameon=True, shadow=True)
    
    plt.tight_layout()
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def create_scalability_summary_plot(df, output_file):
    """Panel 2x2 con resumen completo de escalabilidad"""
    print("📊 Creando panel de resumen de escalabilidad...")
    
    fig = plt.figure(figsize=(14, 10))
    gs = fig.add_gridspec(2, 2, hspace=0.3, wspace=0.3)
    
    # 1. Strong Scaling - Speedup
    ax1 = fig.add_subplot(gs[0, 0])
    N_max = df['N'].max()
    df_strong = df[df['N'] == N_max].copy()
    
    for version, color, label in [('mpi_blocking', COLORS['blocking'], 'Blocking'),
                                  ('mpi_nonblocking', COLORS['nonblocking'], 'Non-Blocking')]:
        df_v = df_strong[df_strong['version'] == version].sort_values('P')
        if len(df_v) > 0:
            ax1.plot(df_v['P'], df_v['speedup'], marker='o', linewidth=2,
                    color=color, label=label, markersize=7)
    
    P_vals = sorted(df_strong['P'].unique())
    ax1.plot(P_vals, P_vals, 'k--', linewidth=1.5, label='Ideal', alpha=0.5)
    ax1.set_xlabel('Procesos (P)')
    ax1.set_ylabel('Speedup')
    ax1.set_title('(a) Strong Scaling - Speedup', fontweight='bold', loc='left')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # 2. Strong Scaling - Eficiencia
    ax2 = fig.add_subplot(gs[0, 1])
    
    for version, color, label in [('mpi_blocking', COLORS['blocking'], 'Blocking'),
                                  ('mpi_nonblocking', COLORS['nonblocking'], 'Non-Blocking')]:
        df_v = df_strong[df_strong['version'] == version].sort_values('P')
        if len(df_v) > 0:
            ax2.plot(df_v['P'], df_v['efficiency'], marker='s', linewidth=2,
                    color=color, label=label, markersize=7)
    
    ax2.axhline(y=100, color='green', linestyle='--', linewidth=1.5, alpha=0.5)
    ax2.set_xlabel('Procesos (P)')
    ax2.set_ylabel('Eficiencia (%)')
    ax2.set_title('(b) Strong Scaling - Eficiencia', fontweight='bold', loc='left')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(0, 110)
    
    # 3. Tiempo de ejecución absoluto
    ax3 = fig.add_subplot(gs[1, 0])
    
    for version, color, label in [('mpi_blocking', COLORS['blocking'], 'Blocking'),
                                  ('mpi_nonblocking', COLORS['nonblocking'], 'Non-Blocking')]:
        df_v = df_strong[df_strong['version'] == version].sort_values('P')
        if len(df_v) > 0:
            ax3.plot(df_v['P'], df_v['time_mean'], marker='D', linewidth=2,
                    color=color, label=label, markersize=7)
            ax3.fill_between(df_v['P'],
                           df_v['time_mean'] - df_v['time_std'],
                           df_v['time_mean'] + df_v['time_std'],
                           color=color, alpha=0.2)
    
    ax3.set_xlabel('Procesos (P)')
    ax3.set_ylabel('Tiempo de Ejecución (s)')
    ax3.set_title('(c) Tiempo Absoluto de Ejecución', fontweight='bold', loc='left')
    ax3.legend()
    ax3.grid(True, alpha=0.3)
    ax3.set_yscale('log')
    
    # 4. Speedup por tamaño de problema
    ax4 = fig.add_subplot(gs[1, 1])
    
    # Promediar speedup por N para cada versión (P>1)
    df_parallel = df[df['P'] > 1].copy()
    
    for version, color, label in [('mpi_blocking', COLORS['blocking'], 'Blocking'),
                                  ('mpi_nonblocking', COLORS['nonblocking'], 'Non-Blocking')]:
        df_v = df_parallel[df_parallel['version'] == version]
        speedup_by_n = df_v.groupby('N')['speedup'].mean().reset_index()
        
        if len(speedup_by_n) > 0:
            ax4.plot(speedup_by_n['N'], speedup_by_n['speedup'],
                    marker='o', linewidth=2, color=color, label=label, markersize=7)
    
    ax4.set_xlabel('Tamaño del Problema (N)')
    ax4.set_ylabel('Speedup Promedio')
    ax4.set_title('(d) Speedup vs Tamaño del Problema', fontweight='bold', loc='left')
    ax4.legend()
    ax4.grid(True, alpha=0.3)
    ax4.set_xscale('log')
    
    # Título general
    fig.suptitle('HPCReto3: Análisis Completo de Escalabilidad\nAutómata Celular con MPI',
                fontsize=16, fontweight='bold', y=0.995)
    
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def main():
    print("="*70)
    print("HPCReto3 - Generador de Gráficas de Presentación")
    print("="*70)
    print()
    
    if len(sys.argv) < 2:
        print("Uso: python3 generate_plots.py <directorio_analisis>")
        print("Ejemplo: python3 generate_plots.py results/analysis")
        sys.exit(1)
    
    analysis_dir = sys.argv[1]
    
    if not os.path.exists(analysis_dir):
        print(f"❌ Error: Directorio no encontrado: {analysis_dir}")
        sys.exit(1)
    
    # Cargar datos
    df = load_speedup_data(analysis_dir)
    
    # Crear directorio de plots
    plots_dir = f"{analysis_dir}/plots_publication"
    os.makedirs(plots_dir, exist_ok=True)
    
    print()
    print("Generando gráficas de alta calidad...")
    print("-" * 70)
    
    # Generar gráficas
    create_publication_speedup_plot(df, f"{plots_dir}/fig1_speedup.png")
    create_efficiency_comparison_plot(df, f"{plots_dir}/fig2_efficiency.png")
    create_performance_improvement_plot(df, f"{plots_dir}/fig3_improvement.png")
    create_scalability_summary_plot(df, f"{plots_dir}/fig4_summary.png")
    
    print()
    print("="*70)
    print("✅ GRÁFICAS GENERADAS")
    print("="*70)
    print()
    print(f"📂 Gráficas guardadas en: {plots_dir}/")
    print()
    print("Archivos listos para informe:")
    print("  📊 fig1_speedup.png       - Speedup vs Procesos")
    print("  📊 fig2_efficiency.png    - Comparación de Eficiencia")
    print("  📊 fig3_improvement.png   - Mejora Non-Blocking vs Blocking")
    print("  📊 fig4_summary.png       - Panel completo de escalabilidad")
    print()

if __name__ == "__main__":
    main()
