# Pre-Deployment Checklist

## ✅ Local Preparation (Completed)

- [x] All source files created (4 implementations)
- [x] Makefile configured for mpicc
- [x] Deployment scripts ready
- [x] Benchmarking scripts ready
- [x] Analysis script with Python ready
- [x] Documentation complete (3 guides)
- [x] Hostfile configured for cluster
- [x] Scripts marked as executable

## ⚠️ Note: Local Compilation Not Possible

**Cannot compile locally** - MPI (mpicc) is not installed on macOS development machine.
- This is **expected and OK**
- Code will compile on AWS cluster which has OpenMPI installed
- Local syntax/logic verified manually

## 📋 Cluster Deployment Steps

### 1. Transfer to Cluster
```bash
# From local machine
cd /Users/isaacpachon/Desktop/Dev/UTP/HPC
tar -czf HPCCasoEstudio3.tar.gz HPCCasoEstudio3/
scp -i ~/.ssh/your-key.pem HPCCasoEstudio3.tar.gz ubuntu@18.224.187.40:~/
```

### 2. Connect and Setup
```bash
# SSH to head node
ssh -i ~/.ssh/your-key.pem ubuntu@18.224.187.40

# Extract
cd ~
tar -xzf HPCCasoEstudio3.tar.gz
cd HPCCasoEstudio3
```

### 3. Verify MPI Installation
```bash
# Check MPI compiler
which mpicc
mpicc --version

# Should show: gcc (Ubuntu ...) with MPI wrapper
```

### 4. Compile
```bash
# Clean and build
make clean
make

# Verify binaries created
ls -lh bin/
```

Expected output:
```
-rwxr-xr-x 1 ubuntu ubuntu  23K ... matrix_mpi_sequential
-rwxr-xr-x 1 ubuntu ubuntu  25K ... matrix_mpi_rowwise
-rwxr-xr-x 1 ubuntu ubuntu  27K ... matrix_mpi_broadcast
-rwxr-xr-x 1 ubuntu ubuntu  28K ... matrix_mpi_nonblocking
```

### 5. Verify Hostfile
```bash
cat hostfile

# Should show:
# localhost slots=2
# worker1 slots=2
# worker2 slots=2

# Test connectivity
ssh worker1 hostname
ssh worker2 hostname
```

### 6. Test MPI Communication
```bash
# Verify 6 processes across cluster
mpirun --hostfile hostfile -np 6 hostname

# Should show 6 lines with hostnames from 3 nodes
```

### 7. Deploy to Workers
```bash
chmod +x scripts/deploy.sh
./scripts/deploy.sh

# Watch for:
# ✓ Success messages for each worker
# ✗ Check for any failed transfers
```

### 8. Quick Test
```bash
# Test smallest configuration
mpirun --hostfile hostfile -np 2 ./bin/matrix_mpi_sequential 256

# Should complete in < 1 second and show timing results
```

### 9. Full Test Run
```bash
# Test one implementation across all process counts
mpirun --hostfile hostfile -np 2 ./bin/matrix_mpi_rowwise 512
mpirun --hostfile hostfile -np 4 ./bin/matrix_mpi_rowwise 512
mpirun --hostfile hostfile -np 6 ./bin/matrix_mpi_rowwise 512

# Verify speedup trend (time should decrease)
```

### 10. Run Benchmarks
```bash
# Full benchmark suite
chmod +x scripts/run_benchmarks.sh
./scripts/run_benchmarks.sh

# Will take 10-15 minutes
# Results saved to results/benchmarks.csv
```

## 🔍 Verification Checklist

After each step, verify:

**Compilation**:
- [ ] No compilation errors
- [ ] 4 binaries in bin/ directory
- [ ] Binaries are executable (chmod +x if needed)

**Deployment**:
- [ ] Binaries copied to both workers
- [ ] No "Permission denied" errors
- [ ] File sizes match between head and workers

**MPI Communication**:
- [ ] `mpirun -np 6 hostname` shows 6 processes
- [ ] Processes distributed across 3 nodes (2 per node)
- [ ] No "unable to connect" errors

**Test Execution**:
- [ ] Programs run without errors
- [ ] Output shows expected matrix size
- [ ] Timing results are reasonable (not 0.0 or negative)
- [ ] Sample results (C[0][0]) are numerical values

**Benchmark Run**:
- [ ] All 36 tests execute
- [ ] No timeouts (300s limit)
- [ ] CSV file created with results
- [ ] Success rate > 90%

## 🚨 Common Issues & Solutions

### Issue: "Matrix size must be divisible by number of processes"
**Solution**: For 6 processes, use sizes: 1024, 2048, 3072 (not 512)

### Issue: "Out of memory"
**Solution**: 
- Use smaller matrix: try 512 or 1024 instead of 2048
- Reduce processes: try -np 4 instead of -np 6
- Check memory: `free -h` on each node

### Issue: "No route to host"
**Solution**:
- Verify workers in hostfile match actual hostnames
- Check SSH keys configured: `ssh worker1 hostname`
- Verify security groups allow internal communication

### Issue: Programs hang/timeout
**Solution**:
- Check if another MPI job is running: `ps aux | grep mpi`
- Kill stale processes: `killall -9 matrix_mpi_rowwise`
- Verify network: `ping worker1`

### Issue: Different results on different runs
**Solution**: 
- This is OK for timing (network variability)
- Results (C[0][0] values) should be identical (deterministic seed)
- If results differ, check for race conditions or uninitialized data

## 📊 Expected Timing Ranges

Rough estimates for AWS t3.micro cluster:

| Matrix Size | Processes | Expected Time | Speedup |
|-------------|-----------|---------------|---------|
| 512×512     | 2         | ~0.5s         | baseline |
| 512×512     | 6         | ~0.3s         | ~1.7x   |
| 1024×1024   | 2         | ~4s           | baseline |
| 1024×1024   | 6         | ~1.5s         | ~2.7x   |
| 2048×2048   | 2         | ~30s          | baseline |
| 2048×2048   | 6         | ~8s           | ~3.8x   |

**Note**: Actual times may vary ±30% based on AWS load

## 📈 After Benchmarks Complete

### Download Results
```bash
# From local machine
scp -i ~/.ssh/your-key.pem -r ubuntu@18.224.187.40:~/HPCCasoEstudio3/results ./results_aws
```

### Analyze Locally
```bash
# Install Python dependencies (if needed)
pip3 install pandas numpy matplotlib seaborn

# Run analysis
cd results_aws
python3 ../HPCCasoEstudio3/scripts/analyze_results.py

# View results
open speedup_comparison.png
open efficiency_comparison.png
cat summary_report.txt
```

### Interpret Results
Look for:
- **Best speedup**: Which implementation performs best?
- **Scalability**: Does speedup increase with more processes?
- **Efficiency drop**: Where does efficiency fall below 80%?
- **Communication overhead**: Is it >20% for any configuration?
- **Load balance**: Are all processes taking equal time?

## ✅ Success Criteria

Project is successful if:
- ✅ All 4 implementations compile without errors
- ✅ At least 90% of benchmark tests complete successfully
- ✅ Speedup > 2x for n=1024, p=6
- ✅ Speedup > 3x for n=2048, p=6
- ✅ No memory errors or crashes
- ✅ Results are reproducible (same input → same output)

## 🎯 Final Deliverables

1. **Code**: 4 working MPI implementations
2. **Data**: benchmarks.csv with 36 test results
3. **Analysis**: Plots and statistical report
4. **Documentation**: Technical explanation of results
5. **Presentation**: Summary of findings and conclusions

---

**Status**: Ready for cluster deployment ✅  
**Next Action**: Transfer to AWS and compile  
**Estimated Time**: 1-2 hours for full workflow
