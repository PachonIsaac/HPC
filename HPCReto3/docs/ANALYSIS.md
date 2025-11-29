# Cellular Automaton Traffic Model - Analysis

## 1. Model Description

### Road Structure
- **N cells** numbered 1 to N (circular road/roundabout)
- Each cell: `0` (empty) or `1` (car present)
- **Periodic boundary conditions**: cell 0 = cell N, cell N+1 = cell 1

### Update Rules
At each timestep `t → t+1`, for each cell `i`:
- **If** current cell has a car (`R_t(i) = 1`) **AND** next cell is empty (`R_t(i+1) = 0`):
  - Car **moves forward** one cell
- **Otherwise**:
  - Car **stays** in place (if blocked) or cell remains empty

### Velocity Metric
```
velocity = (number of cars that moved) / (total number of cars)
```
- Range: [0, 1]
- 0 = complete gridlock (all cars blocked)
- 1 = free flow (all cars moving)

## 2. Truth Tables for Update Rules

### Table 1: Current cell EMPTY (R_t(i) = 0)

| R_t(i-1) | R_t(i) | R_t(i+1) | R_{t+1}(i) | Explanation                    |
|----------|--------|----------|------------|--------------------------------|
| 0        | 0      | 0        | 0          | Empty, no car coming           |
| 0        | 0      | 1        | 0          | Empty, car ahead (not moving)  |
| 1        | 0      | 0        | 1          | **Car from left moves in**     |
| 1        | 0      | 1        | 1          | **Car from left moves in**     |

**Rule**: Cell becomes occupied if left neighbor has a car (that car moves forward).

### Table 2: Current cell OCCUPIED (R_t(i) = 1)

| R_t(i-1) | R_t(i) | R_t(i+1) | R_{t+1}(i) | Explanation                    |
|----------|--------|----------|------------|--------------------------------|
| 0        | 1      | 0        | 0          | **Car moves forward**          |
| 0        | 1      | 1        | 1          | Car blocked, stays             |
| 1        | 1      | 0        | 0          | **Car moves forward**          |
| 1        | 1      | 1        | 1          | Car blocked, stays             |

**Rule**: Car moves forward if next cell is empty; stays if blocked.

### Simplified Combined Rule
```
R_{t+1}(i) = 1  if and only if:
  - (R_t(i) = 1 AND R_t(i+1) = 1)  [blocked car stays]
  OR
  - (R_t(i-1) = 1 AND R_t(i) = 0)  [car from left enters]
```

## 3. Serial Algorithm Pseudocode

### Method 1: Two-Phase (Clear and Simple)
```
Algorithm: TrafficCA_Serial
Input: N (road length), T (timesteps), density (initial car density)
Output: velocities[T] (velocity at each timestep)

// Initialization
old[N] ← initialize_random(density)  // or specific pattern
new[N] ← zeros(N)
total_cars ← count_ones(old)

for t = 1 to T:
    moved_count = 0
    
    // Phase 1: Determine new state for each cell
    for i = 0 to N-1:
        left  = old[(i - 1 + N) mod N]
        curr  = old[i]
        right = old[(i + 1) mod N]
        
        // Apply combined rule
        if (curr == 1 AND right == 1):
            new[i] = 1  // blocked, stays
        else if (left == 1 AND curr == 0):
            new[i] = 1  // car enters from left
        else:
            new[i] = 0  // empty or car moved forward
    
    // Phase 2: Count moves
    for i = 0 to N-1:
        if old[i] == 1 AND new[i] == 0:
            moved_count++
    
    // Update and calculate velocity
    swap(old, new)
    velocities[t] = moved_count / total_cars
    
return velocities
```

### Method 2: Direct Movement Logic (Alternative)
```
for t = 1 to T:
    new[N] ← zeros(N)
    moved = 0
    
    for i = 0 to N-1:
        if old[i] == 1:  // cell has a car
            next = (i + 1) mod N
            if old[next] == 0:  // can move
                new[next] = 1
                moved++
            else:  // blocked
                new[i] = 1
    
    swap(old, new)
    velocity = moved / total_cars
```

**Note**: Method 2 is simpler and directly implements the car movement logic. Both are equivalent.

## 4. Periodic Boundary Conditions

### Implementation
```c
// For cell i, neighbors are:
int left  = (i - 1 + N) % N;  // handles i=0 → wraps to N-1
int right = (i + 1) % N;       // handles i=N-1 → wraps to 0
```

### Example (N=10)
```
Cells:  [0] [1] [2] [3] [4] [5] [6] [7] [8] [9]
         ↑__________________________________|  (periodic)
         |__________________________________↑
```
- Cell 0's left neighbor: cell 9
- Cell 9's right neighbor: cell 0

## 5. Computational Complexity

### Serial Algorithm
- **Time per iteration**: O(N)
  - Loop through N cells: O(N)
  - Each cell: constant time O(1) operations
- **Total time**: O(N × T)
- **Space**: O(N) for two arrays (old and new)

### Memory Access Pattern
- **Sequential**: Good cache locality
- **Read**: Access old[i-1], old[i], old[i+1]
- **Write**: Set new[i]
- **Data dependency**: None between different i (embarrassingly parallel per iteration)

## 6. Verification Strategy

### Correctness Checks
1. **Conservation of cars**: Total cars should remain constant
   ```
   assert(count(old) == total_cars) for all t
   ```

2. **Checksum**: Sum of (position × value)
   ```
   checksum = Σ(i × old[i]) mod LARGE_PRIME
   ```
   - Different states should have different checksums (with high probability)

3. **Velocity bounds**: 
   ```
   0 ≤ velocity ≤ 1
   ```

4. **Determinism**: Same initial state + same seed → same results

### Test Cases
1. **Empty road** (density=0): velocity undefined (or 0)
2. **Full road** (density=1): velocity = 0 (complete gridlock)
3. **Single car** (density=1/N): velocity = 1 (always moves)
4. **Alternating** (1,0,1,0,...): velocity = 0 (all blocked)
5. **Sparse** (density=0.3): velocity ≈ 0.7–0.9

## 7. Expected Behavior

### Phase Transitions
Traffic flow exhibits phase transition behavior:
- **Low density** (ρ < 0.3): Free flow, velocity ≈ 1
- **Medium density** (0.3 < ρ < 0.7): Mixed flow, 0 < velocity < 1
- **High density** (ρ > 0.7): Congestion, velocity → 0
- **Critical density** (ρ ≈ 0.5): Maximum throughput

### Emergent Patterns
- **Traffic jams**: Clusters of stopped cars can form and propagate backwards
- **Periodic solutions**: Some configurations repeat after k timesteps
- **Chaotic behavior**: Small changes in initial conditions → large differences in long-term behavior

## 8. Parameters for Benchmarking

### Recommended Values
```
N (cells):        100, 1000, 10000, 100000, 1000000
T (timesteps):    100, 500, 1000, 5000
density:          0.3, 0.5, 0.7 (interesting regimes)
P (processes):    1, 2, 4, 6 (for MPI)
repetitions:      10 (for statistical significance)
```

### For t3.micro Cluster (1GB RAM)
```
Conservative:
  N = 100,000   (100KB per array)
  T = 1,000
  
Moderate:
  N = 1,000,000 (1MB per array)
  T = 1,000
  
Aggressive:
  N = 10,000,000 (10MB per array) - test first!
  T = 500
```

## 9. Key Insights for Parallelization

### Data Dependencies
- Each cell update depends on **3 values**: left, current, right
- **No cross-iteration dependencies** (double buffer)
- **Spatial locality**: Only adjacent cells needed

### Parallelization Strategy
1. **Domain decomposition**: Divide cells among processes
2. **Ghost cells**: Each process needs boundary cells from neighbors
3. **Communication pattern**: 
   - Send rightmost cell to right neighbor
   - Send leftmost cell to left neighbor
   - Receive from both neighbors
4. **Global reduction**: Sum moved_count across all processes

### Communication vs Computation Ratio
```
Computation: O(N/P) per process
Communication: O(1) messages × O(1) data per iteration
Ratio: O(N/P) → favorable for large N
```

**Conclusion**: Good parallel efficiency expected for N >> P, T >> 1.
