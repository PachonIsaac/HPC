# HPCReto3 - Cellular Automaton Traffic Simulation with MPI

## Project Overview

Implementation and analysis of a 1D cellular automaton traffic flow model with MPI parallelization strategies.

**Course:** High Performance Computing (HPC)  
**Challenge:** Reto 3 - Cellular Automaton Parallelization

## Model Description

### Traffic Rules
- **1D circular road** with N cells (periodic boundary conditions)
- Each cell: `0` (empty) or `1` (car)
- **Update rule:** Car moves forward if next cell is empty, otherwise stays
- **Velocity metric:** `velocity = (cars that moved) / (total cars)`

### Visual Example
```
Initial:  XX.X...XX..XXXXXX...X.XX.XX.XX
t=1:      XX.X.X.XX.X.XXXX.X.X.X.X.X.XX  (velocity=0.39)
t=15:     XX.X.X.X.X.X.XXXX.X.X.X.X.X.XX (velocity=0.67, stabilized)
```
(X = car, . = empty space)

### Key Phenomena
- **Low density** (ρ<0.3): Free flow, velocity≈1.0
- **Medium density** (0.3<ρ<0.7): Mixed flow, 0.3<velocity<0.7
- **High density** (ρ>0.7): Congestion, velocity→0
- **Traffic jams** form and propagate backwards

## Project Structure

```
HPCReto3/
├── src/                          # Source code
│   ├── ca_serial.c               # ✅ Serial implementation
│   ├── ca_mpi_blocking.c         # 🔲 MPI with blocking communication
│   ├── ca_mpi_nonblocking.c      # 🔲 MPI with non-blocking communication
│   └── ca_mpi_master_worker.c    # 🔲 MPI master-worker pattern
├── bin/                          # Compiled binaries
│   └── ca_serial                 # ✅ Working executable
├── scripts/                      # Automation scripts
│   ├── test_serial.sh            # ✅ Test suite (10/10 passed)
│   ├── run_benchmarks.sh         # 🔲 Benchmark execution (10 runs)
│   └── analyze_results.py        # 🔲 Statistical analysis
├── results/                      # Benchmark outputs
│   └── benchmarks.csv            # 🔲 Raw data
├── docs/                         # Documentation
│   └── ANALYSIS.md               # ✅ Detailed algorithm analysis
├── Makefile                      # ✅ Build system
└── README.md                     # This file
```

## Quick Start

### Build
```bash
make              # Build all versions
make ca_serial    # Build only serial version
```

### Run Serial Version
```bash
# Format: ./bin/ca_serial <N> <T> <density> [seed]
./bin/ca_serial 1000 100 0.5 42

# Example outputs:
# N=1000, T=100, density=0.5
#   Mean velocity: 0.9484
#   Time: 0.002 seconds
```

### Test Correctness
```bash
make test-serial
# Runs 10 test cases:
# ✅ Empty road, full road, single car
# ✅ Sparse/medium/dense traffic
# ✅ Large systems, long simulations
# ✅ Determinism, conservation laws
```

## Implementation Status

### ✅ Completed
1. **Analysis** (docs/ANALYSIS.md)
   - Truth tables for CA rules
   - Pseudocode for serial and parallel versions
   - Complexity analysis: O(N×T) serial
   - Verification strategies

2. **Serial Implementation** (src/ca_serial.c)
   - Double buffer update (eliminates race conditions)
   - Periodic boundary conditions
   - Velocity calculation with statistics
   - Conservation verification (checksum)
   - Performance metrics (cell updates/sec)

3. **Testing** (scripts/test_serial.sh)
   - 10/10 tests passed
   - Edge cases: empty, full, single car
   - Phase transitions: sparse→dense traffic
   - Determinism and randomization

### 🔲 In Progress
4. **MPI Parallelization Design**
   - 1D domain decomposition
   - Halo exchange (ghost cells)
   - Global reduction for velocity

5. **MPI Implementations**
   - Blocking: `MPI_Sendrecv` for halo exchange
   - Non-blocking: `MPI_Isend`/`MPI_Irecv` + `MPI_Waitall`
   - Master-worker: Dynamic load balancing

6. **Benchmarking**
   - 10 runs per configuration (professor requirement)
   - Variables: N (100K→1M), T (100→1K), P (1,2,4,6)
   - Metrics: execution time, speedup, efficiency

7. **Analysis**
   - Mean ± std across runs
   - Speedup plots with error bars
   - Strong/weak scaling
   - Communication overhead analysis

## Test Results

### Validation
```
✅ Conservation:   All tests maintain car count (18 cars → 18 cars)
✅ Determinism:    Same seed → identical checksums
✅ Randomization:  Different seeds → different results
✅ Velocity bounds: Always 0 ≤ velocity ≤ 1
✅ Phase behavior:  
   - density=0.3 → velocity=0.992 (free flow)
   - density=0.5 → velocity=0.741 (mixed)
   - density=0.7 → velocity=0.347 (congestion)
```

### Performance (Serial)
```
N=10,000,   T=100:   0.002 sec  (5.0×10⁸ cells/sec)
N=100,000,  T=100:   0.021 sec  (4.7×10⁸ cells/sec)
N=1,000,000, T=100:  0.215 sec  (4.6×10⁸ cells/sec)
```
→ Excellent cache locality, linear scaling with N

## Parallelization Strategy

### Domain Decomposition
```
Process 0: [cells 0...N/P-1]     + right ghost cell
Process 1: [cells N/P...2N/P-1]  + left/right ghost cells
...
Process P-1: [cells (P-1)N/P...N-1] + left ghost cell
```

### Communication Pattern
```
Each timestep:
1. Update interior cells (no communication)
2. Exchange boundary cells:
   - Send rightmost cell to right neighbor
   - Send leftmost cell to left neighbor
   - Receive from both neighbors
3. Update boundary cells with ghost data
4. MPI_Reduce: sum moved_count across processes
```

### Expected Speedup
```
Computation: O(N/P)  (embarrassingly parallel)
Communication: O(1)  (constant per process)
Speedup: S(P) ≈ P / (1 + 2/N)  (near-linear for large N)
```

## Cluster Configuration

### AWS Setup (from HPCCasoEstudio3)
```
Node 0 (head):  172.31.28.209  (2 slots)
Node 1:         172.31.18.36   (2 slots)
Node 2:         172.31.16.182  (2 slots)
Total: 6 processes on 3 × t3.micro (1 vCPU, 1GB RAM each)
```

### Hostfile
```
172.31.28.209 slots=2
172.31.18.36 slots=2
172.31.16.182 slots=2
```

### Run on Cluster
```bash
# Serial baseline
./bin/ca_serial 1000000 1000 0.5 42

# MPI versions (when ready)
mpirun --hostfile hostfile -np 6 ./bin/ca_mpi_blocking 1000000 1000 0.5 42
```

## Benchmark Plan

### Parameters
```
N (cells):     100000, 500000, 1000000
T (timesteps): 100, 500, 1000
density:       0.5 (medium traffic)
P (processes): 1, 2, 4, 6
repetitions:   10 (for statistical significance)
```

### Metrics to Collect
- **Execution time**: Mean ± std across 10 runs
- **Speedup**: S(P) = T(1) / T(P)
- **Efficiency**: E(P) = S(P) / P
- **Coefficient of variation**: CV = std/mean × 100%

### Output Format (CSV)
```csv
version,N,T,density,P,run,execution_time,mean_velocity
serial,100000,100,0.5,1,1,0.0215,0.9034
serial,100000,100,0.5,1,2,0.0218,0.9034
...
mpi_blocking,100000,100,0.5,6,10,0.0041,0.9034
```

## Documentation

### See Also
- **ANALYSIS.md**: Complete algorithm analysis, truth tables, pseudocode
- **MPI_Exercise.txt**: Original challenge statement
- **scripts/test_serial.sh**: Test case definitions

## Next Steps

1. ⏳ Design MPI communication patterns (halo exchange logic)
2. ⏳ Implement `ca_mpi_blocking.c` (MPI_Sendrecv)
3. ⏳ Implement `ca_mpi_nonblocking.c` (MPI_Isend/Irecv)
4. ⏳ Implement `ca_mpi_master_worker.c` (if time permits)
5. ⏳ Create `run_benchmarks.sh` (10 repetitions)
6. ⏳ Create `analyze_results.py` (statistics + plots)
7. ⏳ Execute benchmarks on AWS cluster
8. ⏳ Write final report with performance analysis

## References

- Nagel-Schreckenberg model (traffic CA)
- MPI domain decomposition patterns
- Previous project: HPCCasoEstudio3 (matrix multiplication with MPI)

---

**Status:** Serial version complete and tested ✅  
**Next:** Begin MPI parallelization design
