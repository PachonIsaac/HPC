import pandas as pd

# Cargar datos
df = pd.read_csv('../results/benchmarks.csv')

# Obtener todas las versiones únicas
versiones = df['Version'].unique()
algoritmos = df['Algorithm'].unique()
iteraciones = [1000000, 10000000]

with pd.ExcelWriter('../results/tablas_speedup.xlsx') as writer:
    for version in versiones:
        for algo in algoritmos:
            datos = df[(df['Version'] == version) & (df['Algorithm'] == algo)]
            if datos.empty:
                continue
            # Promedio de tiempo por iteración y threads
            tabla = datos.groupby(['Threads', 'Iterations'])['Time_Seconds'].mean().unstack()
            # Solo mostrar columnas de interés
            tabla = tabla[iteraciones]
            # Fila de promedios
            tabla.loc['Promedio'] = tabla.mean()
            # Fila de speedup (solo si existe versión serial para ese algoritmo)
            serial = df[(df['Version'] == 'Serial') & (df['Algorithm'] == algo)].groupby('Iterations')['Time_Seconds'].mean()
            speedup = serial / tabla.loc['Promedio']
            tabla.loc['Speedup'] = speedup
            # Guardar hoja
            tabla.to_excel(writer, sheet_name=f'{algo}_{version}')

print("Tablas generadas en ../results/tablas_speedup.xlsx")
