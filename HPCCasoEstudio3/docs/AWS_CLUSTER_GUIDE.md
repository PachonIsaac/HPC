# AWS Cluster Setup Guide - HPCCasoEstudio3

## Configuración Actual del Cluster

### Infraestructura
- **Región**: us-east-2 (Ohio)
- **Head Node**: 18.224.187.40 (Elastic IP)
- **Workers**: worker1, worker2 (IPs privadas 172.31.x.x)
- **Instancias**: 3 x t3.micro (1 vCPU, 1GB RAM cada una)
- **OS**: Ubuntu 22.04 LTS
- **MPI**: OpenMPI (ya instalado y verificado)

### Verificación del Cluster

```bash
# Desde tu máquina local, conectar al head node
ssh -i ~/.ssh/your-key.pem ubuntu@18.224.187.40

# En el head node, verificar conectividad con workers
ssh worker1 hostname
ssh worker2 hostname

# Test MPI (debería mostrar 6 procesos)
mpirun --hostfile /path/to/hostfile -np 6 hostname
```

## Pasos para Deployment

### 1. Transferir Código al Cluster

Desde tu máquina local:

```bash
# Comprimir el proyecto
cd /Users/isaacpachon/Desktop/Dev/UTP/HPC
tar -czf HPCCasoEstudio3.tar.gz HPCCasoEstudio3/

# Transferir al head node
scp -i ~/.ssh/your-key.pem HPCCasoEstudio3.tar.gz ubuntu@18.224.187.40:~/

# Conectar al head node
ssh -i ~/.ssh/your-key.pem ubuntu@18.224.187.40

# Descomprimir
cd ~
tar -xzf HPCCasoEstudio3.tar.gz
cd HPCCasoEstudio3
```

### 2. Compilar en el Head Node

```bash
# Verificar que mpicc está disponible
which mpicc
mpicc --version

# Compilar
make clean
make

# Verificar binarios
ls -lh bin/
```

Deberías ver:
- `matrix_mpi_sequential`
- `matrix_mpi_rowwise`
- `matrix_mpi_broadcast`
- `matrix_mpi_nonblocking`

### 3. Configurar Hostfile

El archivo `hostfile` debe estar configurado para tu cluster:

```bash
# Editar hostfile
nano hostfile
```

Contenido:
```
localhost slots=2
worker1 slots=2
worker2 slots=2
```

**Nota**: Si tus workers tienen nombres diferentes, actualízalos aquí.

Para verificar los nombres:
```bash
# Desde head node
cat /etc/hosts
# O
ssh worker1 hostname
ssh worker2 hostname
```

### 4. Desplegar a Workers

```bash
# Hacer ejecutable el script
chmod +x scripts/deploy.sh

# Editar si es necesario (verificar nombres de workers)
nano scripts/deploy.sh

# Ejecutar deployment
./scripts/deploy.sh
```

Este script:
1. Crea directorio `~/HPCCasoEstudio3/bin` en cada worker
2. Copia binarios compilados vía `scp`
3. Verifica la copia exitosa

### 5. Test Inicial

```bash
# Test simple con 2 procesos localmente
mpirun -np 2 ./bin/matrix_mpi_sequential 256

# Test con 6 procesos en el cluster
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_rowwise 512
```

Deberías ver output como:
```
=== MPI Row-wise Distribution ===
Matrix size: 512 x 512
Number of processes: 6
Rows per process: 85

Results:
Total time: X.XXXXXX seconds
...
```

## Ejecución de Benchmarks

### Prueba Rápida

```bash
# Test rápido con tamaños pequeños
mpirun --hostfile hostfile -np 2 ./bin/matrix_mpi_sequential 256
mpirun --hostfile hostfile -np 4 ./bin/matrix_mpi_rowwise 512
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_broadcast 1024
```

### Benchmarks Completos

```bash
# Hacer ejecutable
chmod +x scripts/run_benchmarks.sh

# Ejecutar (tomará varios minutos)
./scripts/run_benchmarks.sh
```

El script probará:
- Tamaños: 512x512, 1024x1024, 2048x2048
- Procesos: 2, 4, 6
- Todas las implementaciones

Resultados se guardan en `results/benchmarks.csv`

### Monitorear Recursos Durante Ejecución

Terminal 1 (ejecutar benchmarks):
```bash
./scripts/run_benchmarks.sh
```

Terminal 2 (monitorear head node):
```bash
watch -n1 'free -h && echo "---" && ps aux | grep matrix_mpi | grep -v grep'
```

Terminal 3 (monitorear worker1):
```bash
ssh worker1 'watch -n1 "free -h"'
```

Terminal 4 (monitorear worker2):
```bash
ssh worker2 'watch -n1 "free -h"'
```

## Análisis de Resultados

### Transferir Resultados a Local

Desde tu máquina local:

```bash
# Descargar resultados
scp -i ~/.ssh/your-key.pem -r ubuntu@18.224.187.40:~/HPCCasoEstudio3/results ./results_aws

# Instalar dependencias Python (si no las tienes)
pip3 install pandas numpy matplotlib seaborn

# Analizar
cd results_aws
python3 ../HPCCasoEstudio3/scripts/analyze_results.py
```

### Análisis en el Cluster

Si tienes Python con las librerías en el cluster:

```bash
# Instalar dependencias (primera vez)
pip3 install --user pandas numpy matplotlib seaborn

# Ejecutar análisis
python3 scripts/analyze_results.py
```

Genera:
- `results/speedup_comparison.png`
- `results/efficiency_comparison.png`
- `results/execution_times.png`
- `results/speedup_heatmap.png`
- `results/summary_report.txt`

## Troubleshooting

### Error: "No route to host" o timeout SSH

```bash
# Verificar security groups en AWS Console
# Asegurar que puerto 22 está abierto entre nodos

# Desde head node, verificar conectividad
ping worker1
ping worker2

# Verificar /etc/hosts
cat /etc/hosts
```

### Error: MPI "unable to fork"

Memoria insuficiente. Soluciones:
```bash
# 1. Reducir número de procesos
mpirun --hostfile hostfile -np 4 ./bin/matrix_mpi_rowwise 512

# 2. Reducir tamaño de matriz
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_rowwise 256

# 3. Verificar memoria disponible
free -h
```

### Error: "Matrix size must be divisible by number of processes"

```bash
# Usar tamaños compatibles:
# Para 2 procesos: cualquier tamaño par
# Para 4 procesos: múltiplos de 4 (512, 1024, 2048)
# Para 6 procesos: múltiplos de 6 (512 no funciona, usar 1024, 2048)

# Solución: usar 1024 o 2048 con 6 procesos
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_rowwise 1024  # ✓
```

### Performance Inesperado

```bash
# 1. Verificar que binarios están optimizados
file bin/matrix_mpi_rowwise  # debería decir "not stripped"

# 2. Recompilar con optimizaciones
make clean
CFLAGS="-O3 -march=native" make

# 3. Verificar latencia de red entre nodos
# Desde head node:
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_sequential 128
# Tiempo debería ser similar a local execution

# 4. Probar diferentes tamaños para encontrar sweet spot
for size in 256 512 1024 2048; do
    echo "Testing size: $size"
    mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_broadcast $size | grep "Total time"
done
```

## Limpieza

```bash
# Limpiar binarios
make clean

# Limpiar resultados antiguos
rm -rf results/*.csv results/*.png results/*.txt

# En workers (desde head node)
ssh worker1 'rm -rf ~/HPCCasoEstudio3/bin/*'
ssh worker2 'rm -rf ~/HPCCasoEstudio3/bin/*'
```

## Tips para Maximizar Performance

1. **Use tamaños de matriz grandes**: 1024+ para ver beneficio de paralelización
2. **Balance procesos por nodo**: 2 procesos por nodo (evitar CPU contention)
3. **Minimice comunicación**: broadcast suele ser mejor que scatter/gather múltiples
4. **Monitor memoria**: Con 1GB RAM, >2048 puede causar swapping
5. **Network latency**: AWS tiene ~1ms latencia entre nodos en misma AZ

## Comandos Útiles

```bash
# Ver procesos MPI en todos los nodos
mpirun --hostfile hostfile -np 6 ps aux | grep mpi

# Matar todos los procesos MPI
killall -9 matrix_mpi_rowwise

# Ver uso de CPU en tiempo real
mpirun --hostfile hostfile -np 6 top -b -n1 | grep matrix

# Benchmark de latencia MPI
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_sequential 128

# Ver logs de MPI (si hay errores)
mpirun --mca btl_base_verbose 30 --hostfile hostfile -np 6 ./bin/matrix_mpi_rowwise 512
```

## Próximos Pasos

1. ✅ Compilar en cluster
2. ✅ Desplegar a workers
3. ✅ Ejecutar test rápido
4. ⏳ Ejecutar benchmarks completos
5. ⏳ Analizar resultados
6. ⏳ Optimizar implementación con mejor performance
7. ⏳ Documentar hallazgos en reporte

## Referencias Rápidas

- MPI Hostfile: [OpenMPI Docs](https://www.open-mpi.org/faq/?category=running#mpirun-hostfile)
- AWS EC2 Networking: [AWS Docs](https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/ec2-instance-network-bandwidth.html)
- Troubleshooting MPI: [MPICH Guide](https://www.mpich.org/static/docs/latest/)
