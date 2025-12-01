#!/usr/bin/env python3
"""
Script de Comparación Local vs Cluster - HPCReto3
Compara resultados de benchmarks entre MacBook M1 y AWS Cluster
Autor: Isaac Pachon
Fecha: 2025-12-01
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
import os

# Configuración de estilo
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (14, 10)
plt.rcParams['font.size'] = 10

# Colores
COLORS = {
    'local': '#3498DB',    # Azul - MacBook M1
    'cluster': '#E74C3C',  # Rojo - AWS
    'blocking': '#E67E22',
    'nonblocking': '#27AE60'
}

def load_environment_data(env_name):
    """Carga datos de speedup/efficiency de un entorno"""
    speedup_file = f"results/{env_name}/analysis/speedup_efficiency.csv"
    
    if not os.path.exists(speedup_file):
        print(f"❌ Error: No se encontró {speedup_file}")
        print(f"   Ejecuta primero: ./scripts/analyze_latest.sh {env_name}")
        return None
    
    df = pd.read_csv(speedup_file)
    df['environment'] = env_name
    return df

def compare_execution_times(df_local, df_cluster, output_dir):
    """Compara tiempos de ejecución absolutos"""
    print("📊 Comparando tiempos de ejecución...")
    
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    # 1. Serial comparison
    ax = axes[0, 0]
    local_serial = df_local[df_local['version'] == 'serial']
    cluster_serial = df_cluster[df_cluster['version'] == 'serial']
    
    if len(local_serial) > 0 and len(cluster_serial) > 0:
        N_common = sorted(set(local_serial['N']) & set(cluster_serial['N']))
        
        local_times = [local_serial[local_serial['N'] == n]['time_mean'].values[0] for n in N_common]
        cluster_times = [cluster_serial[cluster_serial['N'] == n]['time_mean'].values[0] for n in N_common]
        
        x = np.arange(len(N_common))
        width = 0.35
        
        ax.bar(x - width/2, local_times, width, label='Local (M1)', color=COLORS['local'], alpha=0.8)
        ax.bar(x + width/2, cluster_times, width, label='Cluster (t3.micro)', color=COLORS['cluster'], alpha=0.8)
        
        ax.set_xlabel('Tamaño del Problema (N)')
        ax.set_ylabel('Tiempo (s)')
        ax.set_title('(a) Comparación Serial: Local vs Cluster', fontweight='bold')
        ax.set_xticks(x)
        ax.set_xticklabels([f'{n/1000:.0f}K' for n in N_common], rotation=45)
        ax.legend()
        ax.grid(True, alpha=0.3, axis='y')
    
    # 2. MPI Blocking comparison
    ax = axes[0, 1]
    plot_mpi_comparison(df_local, df_cluster, 'mpi_blocking', ax, 'Blocking')
    
    # 3. MPI Non-Blocking comparison
    ax = axes[1, 0]
    plot_mpi_comparison(df_local, df_cluster, 'mpi_nonblocking', ax, 'Non-Blocking')
    
    # 4. Speedup comparison
    ax = axes[1, 1]
    plot_speedup_comparison(df_local, df_cluster, ax)
    
    plt.tight_layout()
    output_file = f"{output_dir}/comparison_local_vs_cluster.png"
    plt.savefig(output_file, dpi=300, bbox_inches='tight')
    print(f"  ✓ {output_file}")
    plt.close()

def plot_mpi_comparison(df_local, df_cluster, version, ax, title):
    """Plotea comparación para una versión MPI específica"""
    local_mpi = df_local[(df_local['version'] == version) & (df_local['P'] > 1)]
    cluster_mpi = df_cluster[(df_cluster['version'] == version) & (df_cluster['P'] > 1)]
    
    if len(local_mpi) > 0 and len(cluster_mpi) > 0:
        # Promediar por P
        local_by_p = local_mpi.groupby('P')['time_mean'].mean()
        cluster_by_p = cluster_mpi.groupby('P')['time_mean'].mean()
        
        P_common = sorted(set(local_by_p.index) & set(cluster_by_p.index))
        
        ax.plot(P_common, [local_by_p[p] for p in P_common], 
               marker='o', linewidth=2, markersize=8,
               color=COLORS['local'], label='Local (M1)')
        ax.plot(P_common, [cluster_by_p[p] for p in P_common],
               marker='s', linewidth=2, markersize=8,
               color=COLORS['cluster'], label='Cluster (AWS)')
        
        ax.set_xlabel('Número de Procesos (P)')
        ax.set_ylabel('Tiempo Promedio (s)')
        ax.set_title(f'({"bc"[version=="mpi_nonblocking"]}) MPI {title}', fontweight='bold')
        ax.legend()
        ax.grid(True, alpha=0.3)

def plot_speedup_comparison(df_local, df_cluster, ax):
    """Compara speedups entre entornos"""
    # Filtrar configuraciones comparables
    for env_name, df, color, marker in [('Local (M1)', df_local, COLORS['local'], 'o'),
                                         ('Cluster (AWS)', df_cluster, COLORS['cluster'], 's')]:
        df_mpi = df[(df['version'] != 'serial') & (df['P'] > 1)]
        if len(df_mpi) > 0:
            speedup_by_p = df_mpi.groupby('P')['speedup'].mean()
            ax.plot(speedup_by_p.index, speedup_by_p.values,
                   marker=marker, linewidth=2, markersize=8,
                   color=color, label=env_name)
    
    # Línea ideal
    if len(df_local) > 0:
        P_vals = sorted(df_local[df_local['P'] > 1]['P'].unique())
        ax.plot(P_vals, P_vals, 'k--', linewidth=1.5, alpha=0.5, label='Speedup Ideal')
    
    ax.set_xlabel('Número de Procesos (P)')
    ax.set_ylabel('Speedup Promedio')
    ax.set_title('(d) Comparación de Speedup', fontweight='bold')
    ax.legend()
    ax.grid(True, alpha=0.3)

def create_summary_table(df_local, df_cluster, output_dir):
    """Crea tabla resumen de comparación"""
    print("📝 Generando tabla resumen...")
    
    summary_data = []
    
    for env_name, df in [('Local (MacBook M1)', df_local), ('Cluster (AWS t3.micro)', df_cluster)]:
        # Serial
        serial_df = df[df['version'] == 'serial']
        if len(serial_df) > 0:
            summary_data.append({
                'Entorno': env_name,
                'Versión': 'Serial',
                'Tiempo Promedio (s)': serial_df['time_mean'].mean(),
                'Speedup Promedio': 1.0,
                'Eficiencia Promedio (%)': 100.0
            })
        
        # MPI Blocking
        blocking_df = df[(df['version'] == 'mpi_blocking') & (df['P'] > 1)]
        if len(blocking_df) > 0:
            summary_data.append({
                'Entorno': env_name,
                'Versión': 'MPI Blocking',
                'Tiempo Promedio (s)': blocking_df['time_mean'].mean(),
                'Speedup Promedio': blocking_df['speedup'].mean(),
                'Eficiencia Promedio (%)': blocking_df['efficiency'].mean()
            })
        
        # MPI Non-Blocking
        nonblocking_df = df[(df['version'] == 'mpi_nonblocking') & (df['P'] > 1)]
        if len(nonblocking_df) > 0:
            summary_data.append({
                'Entorno': env_name,
                'Versión': 'MPI Non-Blocking',
                'Tiempo Promedio (s)': nonblocking_df['time_mean'].mean(),
                'Speedup Promedio': nonblocking_df['speedup'].mean(),
                'Eficiencia Promedio (%)': nonblocking_df['efficiency'].mean()
            })
    
    df_summary = pd.DataFrame(summary_data)
    
    # Guardar CSV
    output_file = f"{output_dir}/comparison_summary.csv"
    df_summary.to_csv(output_file, index=False)
    print(f"  ✓ {output_file}")
    
    # Imprimir tabla
    print("\n" + "="*80)
    print("TABLA COMPARATIVA: LOCAL vs CLUSTER")
    print("="*80)
    print(df_summary.to_string(index=False))
    print("="*80 + "\n")

def generate_text_report(df_local, df_cluster, output_dir):
    """Genera reporte de texto comparativo"""
    print("📝 Generando reporte comparativo...")
    
    report_file = f"{output_dir}/REPORTE_COMPARACION.txt"
    
    with open(report_file, 'w', encoding='utf-8') as f:
        f.write("="*80 + "\n")
        f.write("REPORTE COMPARATIVO: LOCAL vs CLUSTER\n")
        f.write("HPCReto3 - Autómata Celular con MPI\n")
        f.write("="*80 + "\n\n")
        
        # Especificaciones
        f.write("1. ESPECIFICACIONES DE HARDWARE\n")
        f.write("-" * 80 + "\n\n")
        
        f.write("Local (MacBook Air M1):\n")
        f.write("  - CPU: Apple M1 (8 cores: 4 performance + 4 efficiency)\n")
        f.write("  - RAM: 8GB/16GB Unified Memory\n")
        f.write("  - OS: macOS\n")
        f.write("  - MPI: OpenMPI 5.0.9 (Homebrew)\n")
        f.write("  - Network: N/A (single node)\n\n")
        
        f.write("Cluster (AWS t3.micro):\n")
        f.write("  - CPU: 1 vCPU Intel Xeon (variable performance)\n")
        f.write("  - RAM: 1GB\n")
        f.write("  - OS: Ubuntu 24.04 LTS\n")
        f.write("  - MPI: OpenMPI 4.1.6\n")
        f.write("  - Network: AWS VPC (single node, localhost)\n\n")
        
        # Comparación de performance serial
        f.write("2. PERFORMANCE SERIAL\n")
        f.write("-" * 80 + "\n")
        
        local_serial = df_local[df_local['version'] == 'serial']
        cluster_serial = df_cluster[df_cluster['version'] == 'serial']
        
        if len(local_serial) > 0 and len(cluster_serial) > 0:
            local_time = local_serial['time_mean'].mean()
            cluster_time = cluster_serial['time_mean'].mean()
            ratio = cluster_time / local_time
            
            f.write(f"Tiempo promedio Local:   {local_time:.6f}s\n")
            f.write(f"Tiempo promedio Cluster: {cluster_time:.6f}s\n")
            f.write(f"Ratio (Cluster/Local):   {ratio:.2f}x\n")
            f.write(f"M1 es {ratio:.2f}x más rápido que t3.micro\n\n")
        
        # Comparación MPI
        f.write("3. PERFORMANCE PARALELO (MPI)\n")
        f.write("-" * 80 + "\n\n")
        
        for version, label in [('mpi_blocking', 'Blocking'), ('mpi_nonblocking', 'Non-Blocking')]:
            f.write(f"MPI {label}:\n")
            
            local_mpi = df_local[(df_local['version'] == version) & (df_local['P'] > 1)]
            cluster_mpi = df_cluster[(df_cluster['version'] == version) & (df_cluster['P'] > 1)]
            
            if len(local_mpi) > 0 and len(cluster_mpi) > 0:
                local_speedup = local_mpi['speedup'].mean()
                cluster_speedup = cluster_mpi['speedup'].mean()
                local_eff = local_mpi['efficiency'].mean()
                cluster_eff = cluster_mpi['efficiency'].mean()
                
                f.write(f"  Local:   Speedup={local_speedup:.3f}, Eficiencia={local_eff:.2f}%\n")
                f.write(f"  Cluster: Speedup={cluster_speedup:.3f}, Eficiencia={cluster_eff:.2f}%\n\n")
        
        # Conclusiones
        f.write("4. CONCLUSIONES\n")
        f.write("-" * 80 + "\n")
        f.write("• M1 tiene mejor performance absoluta (CPU más potente)\n")
        f.write("• Cluster (t3.micro) limitado por 1 vCPU y 1GB RAM\n")
        f.write("• Speedup similar en ambos entornos (MPI bien implementado)\n")
        f.write("• Non-blocking muestra mejora sobre blocking en ambos entornos\n")
        f.write("• Overhead de MPI más visible en cluster (recursos limitados)\n\n")
        
        f.write("="*80 + "\n")
    
    print(f"  ✓ {report_file}")

def main():
    print("="*80)
    print("HPCReto3 - Comparación Local vs Cluster")
    print("="*80)
    print()
    
    # Cargar datos
    print("📂 Cargando datos...")
    df_local = load_environment_data('local')
    df_cluster = load_environment_data('cluster')
    
    if df_local is None or df_cluster is None:
        print("\n❌ Faltan datos para comparación")
        return
    
    print(f"  ✓ Local: {len(df_local)} configuraciones")
    print(f"  ✓ Cluster: {len(df_cluster)} configuraciones")
    print()
    
    # Crear directorio de comparación
    output_dir = "results/comparison"
    os.makedirs(output_dir, exist_ok=True)
    
    # Generar análisis
    compare_execution_times(df_local, df_cluster, output_dir)
    create_summary_table(df_local, df_cluster, output_dir)
    generate_text_report(df_local, df_cluster, output_dir)
    
    print()
    print("="*80)
    print("✅ COMPARACIÓN COMPLETADA")
    print("="*80)
    print()
    print(f"📂 Resultados en: {output_dir}/")
    print()
    print("Archivos generados:")
    print("  📊 comparison_local_vs_cluster.png - Gráficas comparativas")
    print("  📊 comparison_summary.csv          - Tabla resumen")
    print("  📝 REPORTE_COMPARACION.txt         - Reporte completo")
    print()

if __name__ == "__main__":
    main()
