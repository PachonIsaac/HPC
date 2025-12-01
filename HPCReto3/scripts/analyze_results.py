#!/usr/bin/env python3
"""
Script de Análisis de Resultados - HPCReto3
Procesa benchmarks CSV y genera estadísticas + gráficas
Autor: Isaac Pachon
Fecha: 2025-11-29
"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import sys
import os
from pathlib import Path

# Configuración de estilo
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 10

def load_data(csv_file):
    """Carga y valida el CSV de benchmarks"""
    print(f"📂 Cargando datos desde: {csv_file}")
    
    if not os.path.exists(csv_file):
        print(f"❌ Error: Archivo no encontrado: {csv_file}")
        sys.exit(1)
    
    df = pd.read_csv(csv_file)
    
    # Validar columnas esperadas
    expected_cols = ['version', 'N', 'T', 'density', 'P', 'run', 
                     'execution_time', 'mean_velocity', 'total_cars']
    missing_cols = set(expected_cols) - set(df.columns)
    
    if missing_cols:
        print(f"❌ Error: Columnas faltantes: {missing_cols}")
        sys.exit(1)
    
    print(f"✓ Datos cargados: {len(df)} filas")
    print(f"✓ Versiones: {df['version'].unique()}")
    print(f"✓ Configuraciones N: {sorted(df['N'].unique())}")
    print(f"✓ Procesos P: {sorted(df['P'].unique())}")
    print()
    
    return df

def calculate_statistics(df):
    """Calcula estadísticas por configuración"""
    print("📊 Calculando estadísticas...")
    
    # Agrupar por configuración (version, N, T, density, P)
    grouped = df.groupby(['version', 'N', 'T', 'density', 'P'])
    
    stats = grouped.agg({
        'execution_time': ['mean', 'std', 'min', 'max'],
        'mean_velocity': ['mean', 'std'],
        'total_cars': ['mean', 'std']
    }).reset_index()
    
    # Aplanar nombres de columnas
    stats.columns = ['version', 'N', 'T', 'density', 'P',
                     'time_mean', 'time_std', 'time_min', 'time_max',
                     'velocity_mean', 'velocity_std',
                     'cars_mean', 'cars_std']
    
    # Coeficiente de variación (%)
    stats['time_cv'] = (stats['time_std'] / stats['time_mean'] * 100).round(2)
    
    print(f"✓ {len(stats)} configuraciones únicas analizadas")
    print()
    
    return stats

def calculate_speedup_efficiency(stats):
    """Calcula speedup y eficiencia respecto a serial"""
    print("⚡ Calculando speedup y eficiencia...")
    
    results = []
    
    # Para cada combinación de N, T, density
    for (N, T, density), group in stats.groupby(['N', 'T', 'density']):
        # Tiempo serial como baseline
        serial_time = group[group['version'] == 'serial']['time_mean'].values
        
        if len(serial_time) == 0:
            print(f"⚠️  Warning: No hay tiempo serial para N={N}, T={T}, density={density}")
            continue
        
        serial_time = serial_time[0]
        
        for _, row in group.iterrows():
            version = row['version']
            P = row['P']
            parallel_time = row['time_mean']
            
            # Speedup: S(P) = T_serial / T_parallel
            speedup = serial_time / parallel_time if parallel_time > 0 else 0
            
            # Eficiencia: E(P) = S(P) / P
            efficiency = (speedup / P * 100) if P > 0 else 0
            
            results.append({
                'version': version,
                'N': N,
                'T': T,
                'density': density,
                'P': P,
                'time_mean': parallel_time,
                'time_std': row['time_std'],
                'time_cv': row['time_cv'],
                'serial_time': serial_time,
                'speedup': round(speedup, 3),
                'efficiency': round(efficiency, 2),
                'velocity_mean': row['velocity_mean'],
                'cars_mean': row['cars_mean']
            })
    
    df_results = pd.DataFrame(results)
    print(f"✓ Speedup calculado para {len(df_results)} configuraciones")
    print()
    
    return df_results

def save_summary_tables(stats, speedup_df, output_dir):
    """Guarda tablas resumen en CSV"""
    print("💾 Guardando tablas resumen...")
    
    os.makedirs(output_dir, exist_ok=True)
    
    # 1. Estadísticas completas
    stats_file = f"{output_dir}/statistics.csv"
    stats.to_csv(stats_file, index=False)
    print(f"  ✓ {stats_file}")
    
    # 2. Speedup y eficiencia
    speedup_file = f"{output_dir}/speedup_efficiency.csv"
    speedup_df.to_csv(speedup_file, index=False)
    print(f"  ✓ {speedup_file}")
    
    # 3. Resumen por versión
    version_summary = speedup_df.groupby(['version', 'P']).agg({
        'speedup': ['mean', 'std'],
        'efficiency': ['mean', 'std'],
        'time_mean': ['mean', 'std']
    }).reset_index()
    version_summary.columns = ['version', 'P', 'speedup_mean', 'speedup_std',
                                'efficiency_mean', 'efficiency_std',
                                'time_mean', 'time_std']
    
    version_file = f"{output_dir}/version_summary.csv"
    version_summary.to_csv(version_file, index=False)
    print(f"  ✓ {version_file}")
    
    print()

def plot_strong_scaling(speedup_df, output_dir):
    """Gráfica de Strong Scaling: speedup vs P para N fijo"""
    print("📈 Generando gráfica de Strong Scaling...")
    
    # Filtrar configuración con N más grande (strong scaling típico)
    N_max = speedup_df['N'].max()
    df_strong = speedup_df[speedup_df['N'] == N_max].copy()
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    # Speedup
    for version in df_strong['version'].unique():
        if version == 'serial':
            continue
        df_v = df_strong[df_strong['version'] == version]
        ax1.plot(df_v['P'], df_v['speedup'], marker='o', linewidth=2, 
                label=version.replace('mpi_', '').replace('_', ' ').title())
    
    # Línea ideal
    P_vals = sorted(df_strong['P'].unique())
    ax1.plot(P_vals, P_vals, 'k--', alpha=0.5, label='Speedup Ideal')
    
    ax1.set_xlabel('Número de Procesos (P)', fontsize=12)
    ax1.set_ylabel('Speedup', fontsize=12)
    ax1.set_title(f'Strong Scaling - Speedup (N={N_max:,})', fontsize=14, fontweight='bold')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Eficiencia
    for version in df_strong['version'].unique():
        if version == 'serial':
            continue
        df_v = df_strong[df_strong['version'] == version]
        ax2.plot(df_v['P'], df_v['efficiency'], marker='s', linewidth=2,
                label=version.replace('mpi_', '').replace('_', ' ').title())
    
    ax2.axhline(y=100, color='k', linestyle='--', alpha=0.5, label='Eficiencia 100%')
    ax2.set_xlabel('Número de Procesos (P)', fontsize=12)
    ax2.set_ylabel('Eficiencia (%)', fontsize=12)
    ax2.set_title(f'Strong Scaling - Eficiencia (N={N_max:,})', fontsize=14, fontweight='bold')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    ax2.set_ylim(0, 110)
    
    plt.tight_layout()
    output_file = f"{output_dir}/strong_scaling.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def plot_weak_scaling(speedup_df, output_dir):
    """Gráfica de Weak Scaling: tiempo vs P con N/P constante"""
    print("📈 Generando gráfica de Weak Scaling...")
    
    # Calcular N/P (carga por proceso)
    speedup_df['load_per_process'] = speedup_df['N'] / speedup_df['P']
    
    # Encontrar carga más común
    load_counts = speedup_df['load_per_process'].value_counts()
    if len(load_counts) > 0:
        target_load = load_counts.index[0]
        df_weak = speedup_df[speedup_df['load_per_process'] == target_load].copy()
        
        fig, ax = plt.subplots(figsize=(10, 6))
        
        for version in df_weak['version'].unique():
            if version == 'serial':
                continue
            df_v = df_weak[df_weak['version'] == version].sort_values('P')
            ax.plot(df_v['P'], df_v['time_mean'], marker='o', linewidth=2,
                   label=version.replace('mpi_', '').replace('_', ' ').title())
            
            # Error bars
            ax.fill_between(df_v['P'], 
                          df_v['time_mean'] - df_v['time_std'],
                          df_v['time_mean'] + df_v['time_std'],
                          alpha=0.2)
        
        # Línea ideal (tiempo constante)
        if len(df_weak) > 0:
            time_base = df_weak[df_weak['P'] == df_weak['P'].min()]['time_mean'].mean()
            ax.axhline(y=time_base, color='k', linestyle='--', alpha=0.5, 
                      label=f'Ideal (t={time_base:.4f}s)')
        
        ax.set_xlabel('Número de Procesos (P)', fontsize=12)
        ax.set_ylabel('Tiempo de Ejecución (s)', fontsize=12)
        ax.set_title(f'Weak Scaling (N/P={int(target_load):,} cells/process)', 
                    fontsize=14, fontweight='bold')
        ax.legend()
        ax.grid(True, alpha=0.3)
        
        plt.tight_layout()
        output_file = f"{output_dir}/weak_scaling.png"
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"  ✓ {output_file}")
        plt.close()
    else:
        print("  ⚠️  No hay suficientes datos para weak scaling")

def plot_blocking_vs_nonblocking(speedup_df, output_dir):
    """Comparación directa: Blocking vs Non-Blocking"""
    print("📈 Generando comparación Blocking vs Non-Blocking...")
    
    # Filtrar solo versiones MPI
    df_mpi = speedup_df[speedup_df['version'] != 'serial'].copy()
    
    if len(df_mpi) == 0:
        print("  ⚠️  No hay datos MPI para comparar")
        return
    
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    # Para cada N, comparar tiempos
    N_values = sorted(df_mpi['N'].unique())
    
    # 1. Tiempo de ejecución por N
    ax = axes[0, 0]
    for version in df_mpi['version'].unique():
        df_v = df_mpi[df_mpi['version'] == version]
        # Promediar sobre diferentes P
        df_avg = df_v.groupby('N')['time_mean'].mean()
        ax.plot(df_avg.index, df_avg.values, marker='o', linewidth=2,
               label=version.replace('mpi_', '').replace('_', ' ').title())
    
    ax.set_xlabel('Tamaño del Problema (N)', fontsize=11)
    ax.set_ylabel('Tiempo Promedio (s)', fontsize=11)
    ax.set_title('Tiempo de Ejecución vs Tamaño', fontsize=12, fontweight='bold')
    ax.set_xscale('log')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # 2. Speedup promedio por P
    ax = axes[0, 1]
    for version in df_mpi['version'].unique():
        df_v = df_mpi[df_mpi['version'] == version]
        df_avg = df_v.groupby('P')['speedup'].mean()
        ax.plot(df_avg.index, df_avg.values, marker='s', linewidth=2,
               label=version.replace('mpi_', '').replace('_', ' ').title())
    
    ax.set_xlabel('Número de Procesos (P)', fontsize=11)
    ax.set_ylabel('Speedup Promedio', fontsize=11)
    ax.set_title('Speedup Promedio por Procesos', fontsize=12, fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)
    
    # 3. Mejora de Non-Blocking sobre Blocking (%)
    ax = axes[1, 0]
    df_blocking = df_mpi[df_mpi['version'] == 'mpi_blocking'].copy()
    df_nonblocking = df_mpi[df_mpi['version'] == 'mpi_nonblocking'].copy()
    
    # Merge on configuration
    merged = pd.merge(df_blocking, df_nonblocking, 
                     on=['N', 'T', 'density', 'P'], 
                     suffixes=('_block', '_nonblock'))
    
    if len(merged) > 0:
        merged['improvement'] = ((merged['time_mean_block'] - merged['time_mean_nonblock']) 
                                / merged['time_mean_block'] * 100)
        
        # Agrupar por P
        improvement_by_p = merged.groupby('P')['improvement'].agg(['mean', 'std'])
        
        ax.bar(improvement_by_p.index, improvement_by_p['mean'], 
              yerr=improvement_by_p['std'], capsize=5, alpha=0.7, color='green')
        ax.axhline(y=0, color='k', linestyle='-', linewidth=0.8)
        ax.set_xlabel('Número de Procesos (P)', fontsize=11)
        ax.set_ylabel('Mejora (%)', fontsize=11)
        ax.set_title('Mejora de Non-Blocking sobre Blocking', fontsize=12, fontweight='bold')
        ax.grid(True, alpha=0.3, axis='y')
    
    # 4. Eficiencia por versión
    ax = axes[1, 1]
    for version in df_mpi['version'].unique():
        df_v = df_mpi[df_mpi['version'] == version]
        df_avg = df_v.groupby('P')['efficiency'].mean()
        ax.plot(df_avg.index, df_avg.values, marker='D', linewidth=2,
               label=version.replace('mpi_', '').replace('_', ' ').title())
    
    ax.axhline(y=100, color='k', linestyle='--', alpha=0.5, label='100% Eficiencia')
    ax.set_xlabel('Número de Procesos (P)', fontsize=11)
    ax.set_ylabel('Eficiencia (%)', fontsize=11)
    ax.set_title('Eficiencia Promedio', fontsize=12, fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)
    ax.set_ylim(0, 110)
    
    plt.tight_layout()
    output_file = f"{output_dir}/blocking_vs_nonblocking.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def plot_density_analysis(df, output_dir):
    """Análisis del efecto de densidad en velocidad"""
    print("📈 Generando análisis de densidad...")
    
    # Filtrar diferentes densidades
    densities = sorted(df['density'].unique())
    
    if len(densities) < 2:
        print("  ⚠️  No hay suficientes densidades diferentes para analizar")
        return
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))
    
    # 1. Velocidad promedio vs Densidad
    for version in df['version'].unique():
        df_v = df[df['version'] == version]
        velocity_by_density = df_v.groupby('density')['mean_velocity'].mean()
        ax1.plot(velocity_by_density.index, velocity_by_density.values, 
                marker='o', linewidth=2, markersize=8,
                label=version.replace('mpi_', '').replace('_', ' ').title())
    
    ax1.set_xlabel('Densidad de Tráfico', fontsize=12)
    ax1.set_ylabel('Velocidad Media (cells/timestep)', fontsize=12)
    ax1.set_title('Efecto de Densidad en Velocidad', fontsize=14, fontweight='bold')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # 2. Tiempo de ejecución vs Densidad (para P=4)
    df_p4 = df[df['P'] == 4]
    for version in df_p4['version'].unique():
        if version == 'serial':
            continue
        df_v = df_p4[df_p4['version'] == version]
        time_by_density = df_v.groupby('density')['execution_time'].mean()
        ax2.plot(time_by_density.index, time_by_density.values,
                marker='s', linewidth=2, markersize=8,
                label=version.replace('mpi_', '').replace('_', ' ').title())
    
    ax2.set_xlabel('Densidad de Tráfico', fontsize=12)
    ax2.set_ylabel('Tiempo de Ejecución (s)', fontsize=12)
    ax2.set_title('Tiempo vs Densidad (P=4)', fontsize=14, fontweight='bold')
    ax2.legend()
    ax2.grid(True, alpha=0.3)
    
    plt.tight_layout()
    output_file = f"{output_dir}/density_analysis.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def plot_execution_time_heatmap(speedup_df, output_dir):
    """Heatmap de tiempos de ejecución"""
    print("📈 Generando heatmap de tiempos...")
    
    df_mpi = speedup_df[speedup_df['version'] != 'serial'].copy()
    
    if len(df_mpi) == 0:
        return
    
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
    
    for idx, version in enumerate(['mpi_blocking', 'mpi_nonblocking']):
        df_v = df_mpi[df_mpi['version'] == version]
        
        if len(df_v) == 0:
            continue
        
        # Pivot: N vs P
        pivot = df_v.pivot_table(values='time_mean', index='N', columns='P', aggfunc='mean')
        
        sns.heatmap(pivot, annot=True, fmt='.4f', cmap='YlOrRd', ax=axes[idx],
                   cbar_kws={'label': 'Tiempo (s)'})
        axes[idx].set_title(version.replace('mpi_', '').replace('_', ' ').title(),
                          fontsize=12, fontweight='bold')
        axes[idx].set_xlabel('Número de Procesos (P)', fontsize=11)
        axes[idx].set_ylabel('Tamaño del Problema (N)', fontsize=11)
    
    plt.tight_layout()
    output_file = f"{output_dir}/execution_time_heatmap.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def generate_text_report(stats, speedup_df, output_dir):
    """Genera reporte de texto con resumen ejecutivo"""
    print("📝 Generando reporte de texto...")
    
    report_file = f"{output_dir}/REPORTE_ANALISIS.txt"
    
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write("="*70 + "\n")
        f.write("REPORTE DE ANÁLISIS - HPCReto3: Autómata Celular con MPI\n")
        f.write("="*70 + "\n\n")
        
        # Resumen general
        f.write("1. RESUMEN GENERAL\n")
        f.write("-" * 70 + "\n")
        f.write(f"Total de configuraciones analizadas: {len(stats)}\n")
        f.write(f"Versiones: {', '.join(stats['version'].unique())}\n")
        f.write(f"Tamaños de problema (N): {', '.join(map(str, sorted(stats['N'].unique())))}\n")
        f.write(f"Procesos (P): {', '.join(map(str, sorted(stats['P'].unique())))}\n")
        f.write(f"Timesteps (T): {', '.join(map(str, sorted(stats['T'].unique())))}\n\n")
        
        # Mejor configuración
        f.write("2. MEJORES CONFIGURACIONES\n")
        f.write("-" * 70 + "\n")
        
        best_speedup = speedup_df.nlargest(5, 'speedup')[['version', 'N', 'P', 'speedup', 'efficiency']]
        f.write("Top 5 Speedups:\n")
        f.write(best_speedup.to_string(index=False))
        f.write("\n\n")
        
        best_efficiency = speedup_df.nlargest(5, 'efficiency')[['version', 'N', 'P', 'speedup', 'efficiency']]
        f.write("Top 5 Eficiencias:\n")
        f.write(best_efficiency.to_string(index=False))
        f.write("\n\n")
        
        # Comparación Blocking vs Non-Blocking
        f.write("3. COMPARACIÓN BLOCKING vs NON-BLOCKING\n")
        f.write("-" * 70 + "\n")
        
        df_blocking = speedup_df[speedup_df['version'] == 'mpi_blocking']
        df_nonblocking = speedup_df[speedup_df['version'] == 'mpi_nonblocking']
        
        if len(df_blocking) > 0 and len(df_nonblocking) > 0:
            avg_speedup_block = df_blocking['speedup'].mean()
            avg_speedup_nonblock = df_nonblocking['speedup'].mean()
            avg_eff_block = df_blocking['efficiency'].mean()
            avg_eff_nonblock = df_nonblocking['efficiency'].mean()
            
            f.write(f"Speedup promedio Blocking:     {avg_speedup_block:.3f}\n")
            f.write(f"Speedup promedio Non-Blocking: {avg_speedup_nonblock:.3f}\n")
            f.write(f"Mejora en speedup:             {((avg_speedup_nonblock/avg_speedup_block - 1)*100):.2f}%\n\n")
            
            f.write(f"Eficiencia promedio Blocking:     {avg_eff_block:.2f}%\n")
            f.write(f"Eficiencia promedio Non-Blocking: {avg_eff_nonblock:.2f}%\n")
            f.write(f"Mejora en eficiencia:             {(avg_eff_nonblock - avg_eff_block):.2f}pp\n\n")
        
        # Variabilidad
        f.write("4. ANÁLISIS DE VARIABILIDAD\n")
        f.write("-" * 70 + "\n")
        
        high_cv = stats[stats['time_cv'] > 5.0][['version', 'N', 'P', 'time_cv']]
        if len(high_cv) > 0:
            f.write("⚠️  Configuraciones con alta variabilidad (CV > 5%):\n")
            f.write(high_cv.to_string(index=False))
            f.write("\n\n")
        else:
            f.write("✓ Todas las configuraciones tienen baja variabilidad (CV < 5%)\n\n")
        
        # Strong scaling
        f.write("5. STRONG SCALING ANALYSIS\n")
        f.write("-" * 70 + "\n")
        
        N_max = speedup_df['N'].max()
        df_strong = speedup_df[speedup_df['N'] == N_max].copy()
        
        for version in df_strong['version'].unique():
            if version == 'serial':
                continue
            f.write(f"\n{version.upper()}:\n")
            df_v = df_strong[df_strong['version'] == version].sort_values('P')
            f.write(df_v[['P', 'speedup', 'efficiency', 'time_mean']].to_string(index=False))
            f.write("\n")
        
        f.write("\n" + "="*70 + "\n")
        f.write("Fin del reporte\n")
        f.write("="*70 + "\n")
    
    print(f"  ✓ {report_file}")

def main():
    """Función principal"""
    print("="*70)
    print("HPCReto3 - Script de Análisis de Resultados")
    print("="*70)
    print()
    
    # Verificar argumentos
    if len(sys.argv) < 2:
        print("Uso: python3 analyze_results.py <archivo_csv>")
        print("Ejemplo: python3 analyze_results.py results/benchmarks_20251129_143022.csv")
        sys.exit(1)
    
    csv_file = sys.argv[1]
    
    # Directorio de salida
    output_dir = "results/analysis"
    os.makedirs(output_dir, exist_ok=True)
    
    # Pipeline de análisis
    df = load_data(csv_file)
    stats = calculate_statistics(df)
    speedup_df = calculate_speedup_efficiency(stats)
    
    # Guardar tablas
    save_summary_tables(stats, speedup_df, output_dir)
    
    # Generar gráficas
    print("📊 Generando gráficas...")
    plot_strong_scaling(speedup_df, output_dir)
    plot_weak_scaling(speedup_df, output_dir)
    plot_blocking_vs_nonblocking(speedup_df, output_dir)
    plot_density_analysis(df, output_dir)
    plot_execution_time_heatmap(speedup_df, output_dir)
    print()
    
    # Reporte de texto
    generate_text_report(stats, speedup_df, output_dir)
    print()
    
    # Resumen final
    print("="*70)
    print("✅ ANÁLISIS COMPLETADO")
    print("="*70)
    print()
    print(f"📂 Resultados guardados en: {output_dir}/")
    print()
    print("Archivos generados:")
    print("  📊 statistics.csv              - Estadísticas por configuración")
    print("  📊 speedup_efficiency.csv      - Speedup y eficiencia")
    print("  📊 version_summary.csv         - Resumen por versión")
    print("  📈 strong_scaling.png          - Gráficas de strong scaling")
    print("  📈 weak_scaling.png            - Gráficas de weak scaling")
    print("  📈 blocking_vs_nonblocking.png - Comparación entre versiones")
    print("  📈 density_analysis.png        - Análisis de densidad")
    print("  📈 execution_time_heatmap.png  - Heatmap de tiempos")
    print("  📝 REPORTE_ANALISIS.txt        - Reporte textual completo")
    print()

if __name__ == "__main__":
    main()
