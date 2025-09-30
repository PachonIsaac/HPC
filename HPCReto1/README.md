# HPCReto1: Cálculo de PI con Monte Carlo Needles y Dartboard

Este proyecto implementa y analiza dos algoritmos para el cálculo de PI:
- Monte Carlo Needles
- Monte Carlo Dartboard

Incluye versiones seriales y paralelas (usando hilos y procesos), análisis de desempeño y comparación de resultados.

## Estructura
- `src/` : Código fuente en C
- `Makefile` : Compilación de los programas
- `scripts/` : Scripts para pruebas y benchmarks
- `benchmarks.csv` : Resultados de desempeño

## Algoritmos
- **Monte Carlo Needles**: Simulación de lanzamiento de agujas para estimar PI.
- **Monte Carlo Dartboard**: Simulación de lanzamiento de dardos en un tablero para estimar PI.

## Paralelización
- **Threads**: Usando `pthread`.
- **Procesos**: Usando `fork`.

## Compilación

Desde la raíz del proyecto, ejecuta:

```bash
make
```
Esto compilará todos los ejecutables y los dejará en la carpeta `bin/`.

## Ejecución de algoritmos

Todos los algoritmos pueden recibir parámetros desde la línea de comandos:

- **Serial:**
	```bash
	./bin/pi_needles_serial <iteraciones>
	./bin/pi_needles_serial_opt <iteraciones>
	./bin/pi_dartboard_serial <iteraciones>
	./bin/pi_dartboard_serial_opt <iteraciones>
	```
- **Threads:**
	```bash
	./bin/pi_needles_thread <iteraciones> <hilos>
	./bin/pi_dartboard_thread <iteraciones> <hilos>
	```
- **Fork:**
	```bash
	./bin/pi_needles_fork <iteraciones> <procesos>
	./bin/pi_dartboard_fork <iteraciones> <procesos>
	```

Ejemplo:
```bash
./bin/pi_needles_serial_opt 10000000
./bin/pi_dartboard_serial_opt 40000000
./bin/pi_needles_thread 10000000 4
./bin/pi_dartboard_fork 40000000 8
```

## Pruebas de desempeño y benchmark

### Prueba rápida
En la carpeta `scripts/` hay dos scripts para comparar el desempeño de cada algoritmo (incluyendo las versiones optimizadas):

- Para Needles:
	```bash
	cd scripts
	./test_needles.sh
	```
- Para Dartboard:
	```bash
	cd scripts
	./test_dartboard.sh
	```

### Benchmark automático
Para ejecutar todas las combinaciones de algoritmos, iteraciones y hilos/procesos, y guardar los resultados en `benchmarksReto1.csv`:

```bash
cd scripts
./run_benchmarks.sh
```
Esto ejecuta cada algoritmo (incluyendo las versiones optimizadas) con:
- Iteraciones: 10,000,000; 40,000,000; 80,000,000; 100,000,000
- Hilos/procesos: 2, 4, 8, 12
- 5 repeticiones por combinación
Los resultados se guardan en `benchmarksReto1.csv`.

## Resultados

Los resultados de los benchmarks se guardan automáticamente en el archivo `benchmarksReto1.csv` para análisis y comparación. Puedes abrirlo con cualquier editor de texto o procesador de hojas de cálculo.
