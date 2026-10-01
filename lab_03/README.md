# Lab 03 — Multithreaded Matrix Multiplication

Laboratory work on operating system threads and multithreaded data processing.

The program performs multiplication of two matrices containing complex numbers using POSIX Threads (`pthread`). The computation is divided between several worker threads, while the maximum number of simultaneously running worker threads is specified through a command-line argument.

The laboratory work also includes an experimental study of how execution time, speedup, and parallel efficiency depend on:

- the number of worker threads;
- the size of the input matrices.

---

## 1. Objective

The objective of this laboratory work is to gain practical experience with operating system threads and thread synchronization mechanisms.

The program must:

- process data using multiple threads;
- use standard operating system thread facilities;
- allow the maximum number of simultaneously running threads to be specified through a command-line option;
- demonstrate the actual number of threads using standard operating system tools;
- investigate the dependence of speedup and efficiency on the number of threads and the input data size.

The implementation uses the POSIX Threads API (`pthread`).

---

## 2. Task

### Variant 6

The task is to multiply two matrices containing complex numbers.

Let:

```text
A — matrix of size m × n
B — matrix of size n × p
```

Their product is:

```text
C — matrix of size m × p
```

Matrix multiplication is possible only if the number of columns of the first matrix is equal to the number of rows of the second matrix:

```text
cols(A) = rows(B)
```

Each element of the resulting matrix is calculated as:

```text
C[i][j] = A[i][0] * B[0][j]
        + A[i][1] * B[1][j]
        + ...
        + A[i][n - 1] * B[n - 1][j]
```

In general:

```text
C[i][j] = sum(A[i][k] * B[k][j]), k = 0 ... n - 1
```

---

## 3. Complex Numbers

Each matrix element is a complex number:

```text
z = a + bi
```

where:

- `a` is the real part;
- `b` is the imaginary part;
- `i` is the imaginary unit.

A complex number is represented by the following structure:

```c
typedef struct Complex {
    int re;
    int im;
} comp;
```

The `re` field stores the real part and `im` stores the imaginary part.

### Complex multiplication

For two complex numbers:

```text
z1 = a + bi
z2 = c + di
```

their product is:

```text
z1 * z2 = (ac - bd) + (ad + bc)i
```

The program implements this operation in a separate `multiply()` function.

---

## 4. Matrix Representation

Matrices are allocated dynamically.

A matrix is represented as a pointer to an array of pointers:

```c
comp **matrix;
```

First, memory is allocated for the array of row pointers:

```c
matrix = malloc(sizeof(comp *) * rows);
```

Then memory is allocated for each individual row:

```c
for (long i = 0; i < rows; i++) {
    matrix[i] = malloc(sizeof(comp) * cols);
}
```

Therefore, a matrix element can be accessed using the usual two-dimensional notation:

```c
matrix[i][j]
```

Three matrices are used during the computation:

- `A` — the first input matrix;
- `B` — the second input matrix;
- `C` — the resulting matrix.

---

## 5. Input Modes

The program supports two input modes.

### Automatic mode

By default, matrix elements are generated automatically.

Example:

```bash
./main -t 4
```

The user only enters the dimensions of both matrices.

The matrix elements are then filled with pseudo-random complex numbers.

This mode is primarily used for performance measurements because manually entering large matrices would be impractical.

### Manual mode

The `-m` option enables manual matrix input:

```bash
./main -t 4 -m
```

In this mode, the user enters all elements of both matrices manually.

Complex numbers are entered in the following form:

```text
1+2i
3-4i
5+0i
```

The resulting matrix is printed after the multiplication is completed.

Manual mode is useful for checking the correctness of the algorithm on small matrices.

---

## 6. Command-Line Arguments

The program is started using:

```bash
./main -t <threads>
```

where `<threads>` specifies the maximum number of worker threads.

For example:

```bash
./main -t 8
```

requests up to 8 worker threads.

Manual input can be enabled using:

```bash
./main -t 8 -m
```

The program validates the command-line arguments before starting the computation.

The number of threads is parsed using `strtol()`.

Invalid values such as zero, negative numbers, or non-numeric strings are rejected.

---

## 7. Number of Worker Threads

The requested number of threads can be greater than the number of rows in the resulting matrix.

Creating more worker threads than rows would not be useful because each worker is assigned a range of rows.

Therefore, the actual number of worker threads is calculated as:

```text
threads_count = min(max_threads, rows)
```

For example, if the resulting matrix contains only 3 rows and the program is started as:

```bash
./main -t 10
```

only 3 worker threads are created.

---

## 8. POSIX Threads

The program uses POSIX Threads (`pthread`).

A new thread is created using:

```c
pthread_create()
```

and the main thread waits for worker completion using:

```c
pthread_join()
```

Conceptually, thread creation has the following form:

```c
pthread_create(
    &thread,
    NULL,
    worker,
    &data
);
```

The new thread starts executing the `worker()` function.

The main thread does not perform matrix multiplication itself.

Its main responsibilities are:

1. reading and validating input;
2. allocating matrices;
3. preparing worker data;
4. creating worker threads;
5. waiting for them to finish;
6. measuring execution time;
7. releasing allocated resources.

---

## 9. Thread Data

Each worker receives its own structure containing the information required for the computation.

The structure contains:

```c
typedef struct {
    comp **A;
    comp **B;
    comp **C;

    long cols_a;
    long cols_b;

    long start_row;
    long end_row;
} TD;
```

The pointers `A`, `B`, and `C` refer to the same matrices for all worker threads.

However, each worker receives its own row interval:

```text
[start_row, end_row)
```

This determines which rows of matrix `C` the thread is responsible for calculating.

---

## 10. Work Distribution

The rows of the resulting matrix are divided between worker threads.

For worker `i`:

```c
data[i].start_row = i * rows / threads_count;
data[i].end_row = (i + 1) * rows / threads_count;
```

This divides the rows as evenly as possible.

For example, suppose there are 10 rows and 4 worker threads.

The intervals will be:

```text
Thread 0: rows [0, 2)
Thread 1: rows [2, 5)
Thread 2: rows [5, 7)
Thread 3: rows [7, 10)
```

Each row belongs to exactly one worker.

The intervals do not overlap.

---

## 11. Worker Algorithm

Each worker computes the elements of its assigned rows of the resulting matrix.

The general algorithm is:

```text
for every assigned row i:
    for every column j of B:
        result = 0

        for every column k of A:
            result += A[i][k] * B[k][j]

        C[i][j] = result
```

The worker function has the following general structure:

```c
void *worker(void *arg) {
    TD *data = (TD *)arg;

    for (long i = data->start_row;
         i < data->end_row;
         i++) {

        for (long j = 0;
             j < data->cols_b;
             j++) {

            comp result = {0, 0};

            for (long k = 0;
                 k < data->cols_a;
                 k++) {

                comp product =
                    multiply(
                        data->A[i][k],
                        data->B[k][j]
                    );

                result.re += product.re;
                result.im += product.im;
            }

            data->C[i][j] = result;
        }
    }

    return NULL;
}
```

---

## 12. Why a Mutex Is Not Required

All worker threads have access to the same matrices, but their access patterns are different.

Matrices `A` and `B` are only read during multiplication.

Therefore, several threads can safely read them simultaneously.

Matrix `C` is modified by the worker threads, but every thread operates on its own non-overlapping range of rows.

For example:

```text
Thread 0 -> C[0 ... 249]
Thread 1 -> C[250 ... 499]
Thread 2 -> C[500 ... 749]
Thread 3 -> C[750 ... 999]
```

No two threads write to the same matrix element.

Therefore, there is no data race between worker threads during matrix multiplication.

A mutex is consequently not required for matrix elements.

The synchronization point occurs when the main thread calls `pthread_join()` and waits for all worker threads to finish.

---

## 13. Thread Creation and Completion

It is important to distinguish between:

```c
pthread_create()
```

and:

```c
pthread_join()
```

`pthread_create()` creates a new thread and allows it to begin execution.

It does not wait until that thread finishes.

Therefore, after several calls to `pthread_create()`, multiple workers may execute concurrently.

After all workers have been created, the main thread calls:

```c
pthread_join()
```

for every worker.

This guarantees that the program does not continue to the final stage until all matrix rows have been calculated.

---

## 14. Compilation

The project can be compiled using:

```bash
gcc src/*.c -o main -pthread
```

The `-pthread` option enables the appropriate POSIX thread compilation and linking configuration.

---

## 15. Example

The program can be started with four worker threads:

```bash
./main -t 4
```

Example dimensions:

```text
Enter a size of the first matrix: 1000 1000
Enter a size of the second matrix: 1000 1000
```

The program generates the matrices automatically, performs the multiplication, and prints the measured execution time.

For manual input:

```bash
./main -t 4 -m
```

---

## 16. Correctness Test

Before performing large benchmarks, the implementation was tested using small matrices with manually specified complex values.

Matrix `A`:

```text
1+2i  2+0i  0+1i
3-1i  1+1i  2+2i
0+2i  4-1i  1+0i
```

Matrix `B`:

```text
2+0i  1+1i  3-1i
1-1i  2+0i  0+2i
3+2i  1-1i  2+1i
```

Expected result:

```text
2+5i   4+4i   4+11i
10+8i  10+4i  8+2i
6+1i   7-1i   6+15i
```

The program produced the same result when executed with different thread counts.

The result remained identical for runs using 1, 2, 3, and more requested worker threads.

This confirms that dividing the computation between threads does not change the mathematical result.

---

## 17. Demonstrating Operating System Threads

The number of threads used by the program was inspected using standard operating system tools.

The experiments were performed on macOS.

The process ID can be obtained using:

```bash
pgrep main
```

The threads belonging to the process can then be displayed using:

```bash
ps -M -p <PID>
```

or directly:

```bash
ps -M -p $(pgrep main)
```

When the program was started with:

```bash
./main -t 10
```

the operating system displayed:

```text
1 main thread
10 worker threads
```

Therefore, 11 thread entries were visible for the process.

All threads have the same process ID because they belong to the same process and share its address space.

The worker threads were observed in the running/runnable state while performing the CPU-intensive matrix multiplication.

At the same time, the main thread was mostly waiting for the workers in `pthread_join()`.

This confirms that the program actually performs the computation using multiple operating system threads.

---

# Performance Study

## 18. What Was Measured

Two main experiments were performed.

### Experiment 1 — Number of Threads

The matrix size was fixed at:

```text
A = 1500 × 1500
B = 1500 × 1500
C = 1500 × 1500
```

The number of worker threads was changed:

```text
1, 2, 4, 6, 8, 10, 12, 16
```

The goal was to determine how increasing the number of worker threads affects:

- execution time;
- speedup;
- parallel efficiency.

### Experiment 2 — Input Size

The number of worker threads was fixed at either:

```text
1 thread
10 threads
```

and the matrix size was changed:

```text
100 × 100
500 × 500
1000 × 1000
1500 × 1500
```

The goal was to determine how the size of the computational workload affects the benefits of multithreading.

---

## 19. Execution Time Measurement

Execution time is measured using the monotonic system clock.

The timer starts immediately before worker thread creation:

```c
clock_gettime(CLOCK_MONOTONIC, &start);
```

The timer stops after all worker threads have been joined:

```c
clock_gettime(CLOCK_MONOTONIC, &finish);
```

The elapsed time is calculated as:

```c
double elapsed =
    (finish.tv_sec - start.tv_sec) +
    (finish.tv_nsec - start.tv_nsec) /
    1000000000.0;
```

Therefore, the measured interval includes:

```text
thread creation
       |
       v
matrix multiplication
       |
       v
waiting for workers with pthread_join()
       |
       v
timer stop
```

The measured time does **not** include:

- reading matrix dimensions;
- matrix memory allocation;
- random matrix generation;
- manual input;
- result printing.

This allows the experiment to focus on the parallel computation and the overhead associated with managing the worker threads.

`CLOCK_MONOTONIC` is used because it is suitable for measuring elapsed time and is not affected by changes to the system wall clock.

---

## 20. Performance Metrics

Three main metrics are used to evaluate the program.

### Execution Time

`T(p)` is the execution time when `p` worker threads are used.

For example:

```text
T(1)  — execution time with 1 worker thread
T(10) — execution time with 10 worker threads
```

Lower execution time means better performance.

---

### Speedup

Speedup shows how many times faster the parallel version is compared with the one-thread version.

It is calculated as:

```text
S(p) = T(1) / T(p)
```

For example, for the `1500 × 1500` benchmark:

```text
T(1)  = 25.383677 s
T(10) = 4.605120 s
```

Therefore:

```text
S(10) = 25.383677 / 4.605120
      ≈ 5.512
```

This means that the ten-thread version completed the computation approximately **5.51 times faster** than the one-thread version.

In an ideal parallel system:

```text
S(p) = p
```

For example, the ideal speedup with 10 threads would be `10×`.

---

### Parallel Efficiency

Parallel efficiency shows how effectively the available worker threads are being used compared with ideal linear speedup.

It is calculated as:

```text
E(p) = S(p) / p
```

or as a percentage:

```text
E(p) = S(p) / p × 100%
```

For 10 worker threads:

```text
S(10) = 5.512
p     = 10

E(10) = 5.512 / 10 × 100%
      ≈ 55.1%
```

Therefore, the measured ten-thread efficiency is approximately **55.1%**.

An ideal parallel system would have:

```text
E(p) = 100%
```

---

## 21. Test System

Performance experiments were performed on the following system:

```text
Computer: MacBook Pro
Processor: Apple M5
CPU cores: 10
Performance cores: 4
Efficiency cores: 6
Memory: 24 GB
Operating system: macOS
```

The processor contains 10 physical CPU cores divided into two classes:

```text
4 Performance cores
6 Efficiency cores
```

The cores therefore do not necessarily provide identical computational performance.

This is important when interpreting the benchmark results.

---

# Experiment 1 — Dependence on the Number of Threads

## 22. Benchmark Results

Both matrices had dimensions:

```text
1500 × 1500
```

The following worker thread counts were tested:

```text
1, 2, 4, 6, 8, 10, 12, 16
```

The results were:

| Worker Threads | Average Time | Speedup | Efficiency |
|---:|---:|---:|---:|
| 1 | 25.383677 s | 1.000× | 100.0% |
| 2 | 12.541255 s | 2.024× | 101.2% |
| 4 | 6.794006 s | 3.736× | 93.4% |
| 6 | 5.612017 s | 4.523× | 75.4% |
| 8 | 4.930257 s | 5.149× | 64.4% |
| 10 | 4.605120 s | 5.512× | 55.1% |
| 12 | 4.624499 s | 5.489× | 45.7% |
| 16 | 4.644194 s | 5.466× | 34.2% |

---

## 23. One Thread

With one worker thread:

```text
T(1) = 25.383677 s
```

This is the baseline measurement.

By definition:

```text
S(1) = 1.000×
E(1) = 100%
```

All other speedup values are calculated relative to this execution time.

---

## 24. Two Threads

With two worker threads:

```text
T(2) = 12.541255 s
```

Speedup:

```text
S(2) = 25.383677 / 12.541255
     ≈ 2.024×
```

Efficiency:

```text
E(2) = 2.024 / 2 × 100%
     ≈ 101.2%
```

The measured efficiency is slightly above 100%.

This does not mean that two CPU cores are physically performing more than twice the amount of work.

Real benchmark results are affected by factors such as:

- operating system scheduling;
- cache state;
- CPU frequency behavior;
- background processes;
- normal measurement variation.

The difference from 100% is small, so this result can be interpreted as approximately ideal scaling for this measurement.

---

## 25. Four Threads

With four worker threads:

```text
T(4) = 6.794006 s
S(4) = 3.736×
E(4) = 93.4%
```

This is still relatively close to ideal parallel scaling.

The matrix multiplication contains enough computational work for thread-management overhead to remain small relative to useful computation.

---

## 26. Six Threads

With six worker threads:

```text
T(6) = 5.612017 s
S(6) = 4.523×
E(6) = 75.4%
```

Execution time continues to decrease.

However, the speedup no longer grows proportionally to the number of worker threads.

---

## 27. Eight Threads

With eight worker threads:

```text
T(8) = 4.930257 s
S(8) = 5.149×
E(8) = 64.4%
```

Adding worker threads still improves execution time, but each additional worker provides a smaller performance improvement.

---

## 28. Ten Threads

With ten worker threads:

```text
T(10) = 4.605120 s
S(10) = 5.512×
E(10) = 55.1%
```

The tested computer contains 10 physical CPU cores.

However, these cores are heterogeneous:

```text
4 Performance cores
6 Efficiency cores
```

Therefore, ten worker threads do not correspond to ten identical computational units.

---

## 29. More Threads Than CPU Cores

Increasing the number of worker threads beyond the number of physical CPU cores did not provide additional performance.

For 12 threads:

```text
T(12) = 4.624499 s
S(12) = 5.489×
E(12) = 45.7%
```

For 16 threads:

```text
T(16) = 4.644194 s
S(16) = 5.466×
E(16) = 34.2%
```

Compare these values with 10 threads:

```text
10 threads: 4.605120 s
12 threads: 4.624499 s
16 threads: 4.644194 s
```

The differences are very small.

Therefore, it is more appropriate to describe this region as a **performance plateau** rather than claim that one exact thread count is always optimal.

After approximately 10 workers, adding more threads produced no meaningful additional speedup in these measurements.

---

## 30. Thread Scaling Analysis

The results can be divided into several regions.

### 1–4 threads

The program scales very well.

At four threads:

```text
S(4) = 3.736×
E(4) = 93.4%
```

This is relatively close to the ideal four-thread speedup.

### 4–10 threads

Execution time continues to improve, but the benefit of each additional worker becomes smaller.

Speedup increases from:

```text
3.736× at 4 threads
```

to:

```text
5.512× at 10 threads
```

At the same time, efficiency decreases from:

```text
93.4%
```

to:

```text
55.1%
```

### More than 10 threads

No meaningful additional speedup was observed.

The execution times for 10, 12, and 16 worker threads are all approximately:

```text
4.6 s
```

The system contains 10 CPU cores.

When the number of CPU-bound worker threads exceeds the available CPU resources, several runnable workers must share CPU time.

The operating system scheduler must manage these threads, while the workers also compete for shared hardware resources such as caches and memory bandwidth.

Therefore, adding more threads does not necessarily make the program faster.

---

## 31. Oversubscription Experiment

An additional extreme experiment was performed with:

```text
Matrix size: 1500 × 1500
Requested worker threads: 1000
```

The measured execution time was approximately:

```text
5.92 s
```

For comparison:

```text
10 threads   -> approximately 4.61 s
1000 threads -> approximately 5.92 s
```

The system also became noticeably less responsive during the 1000-thread experiment.

This demonstrates **CPU oversubscription**.

Creating 1000 software threads does not create 1000 physical CPU cores.

Instead, a very large number of runnable threads compete for only 10 physical CPU cores.

This introduces additional:

- scheduling overhead;
- context switching;
- thread-management overhead;
- cache competition;
- memory subsystem competition.

Therefore:

> More threads do not automatically mean better performance.

---

# Experiment 2 — Dependence on Input Size

## 32. Benchmark Results

The second experiment compared one worker thread with ten worker threads for different square matrix sizes.

The following sizes were tested:

```text
100 × 100
500 × 500
1000 × 1000
1500 × 1500
```

The results were:

| Matrix Size | 1 Thread | 10 Threads | Speedup | Efficiency |
|---:|---:|---:|---:|---:|
| 100 × 100 | 0.013432 s | 0.003832 s | 3.505× | 35.05% |
| 500 × 500 | 0.562025 s | 0.113521 s | 4.951× | 49.51% |
| 1000 × 1000 | 5.875144 s | 0.992891 s | 5.917× | 59.17% |
| 1500 × 1500 | 25.383677 s | 4.605120 s | 5.512× | 55.12% |

For the `100 × 100`, `500 × 500`, and `1000 × 1000` cases, the table contains individual benchmark measurements.

For the `1500 × 1500` case, the values are the averaged measurements from the main thread-scaling experiment.

Small differences between individual benchmark results should therefore not be overinterpreted.

---

## 33. Computational Complexity

The classical dense matrix multiplication algorithm contains three nested loops.

For square matrices of size `N × N`, the number of inner-loop iterations grows approximately as:

```text
N³
```

Therefore, its computational complexity is:

```text
O(N³)
```

For example:

```text
100³  = 1,000,000
1000³ = 1,000,000,000
```

A `1000 × 1000` multiplication therefore contains approximately 1000 times as many inner-loop iterations as a `100 × 100` multiplication.

This is important because the cost of creating and joining threads does not grow at the same rate as the matrix computation itself.

---

## 34. Small Matrices

For `100 × 100` matrices:

```text
T(1)  = 0.013432 s
T(10) = 0.003832 s

S(10) = 3.505×
E(10) = 35.05%
```

The computation itself is very short.

As a result, fixed parallelization costs such as:

- creating worker threads;
- scheduling them;
- waiting for them using `pthread_join()`;

represent a relatively large fraction of the total execution time.

Multithreading still improved the measured execution time, but the ten worker threads were not used very efficiently.

---

## 35. Medium-Sized Matrices

For `500 × 500` matrices:

```text
T(1)  = 0.562025 s
T(10) = 0.113521 s

S(10) = 4.951×
E(10) = 49.51%
```

The amount of useful computation is significantly larger than for `100 × 100`.

Therefore, thread-management overhead represents a smaller fraction of total execution time.

The benefits of parallel execution become more significant.

---

## 36. Large Matrices

For `1000 × 1000` matrices:

```text
T(1)  = 5.875144 s
T(10) = 0.992891 s

S(10) = 5.917×
E(10) = 59.17%
```

This produced the highest measured ten-thread efficiency among the tested input sizes.

For `1500 × 1500` matrices:

```text
T(1)  = 25.383677 s
T(10) = 4.605120 s

S(10) = 5.512×
E(10) = 55.12%
```

The small decrease compared with the `1000 × 1000` measurement does not mean that larger matrices inherently parallelize worse.

The results can also be affected by:

- CPU cache behavior;
- memory access patterns;
- memory bandwidth;
- operating system scheduling;
- heterogeneous CPU cores;
- background system activity;
- normal benchmark variation.

The important general observation is that very small workloads make parallelization overhead relatively expensive, while larger workloads provide enough useful computation to benefit substantially from multiple worker threads.

---

## 37. Why Speedup Is Not Equal to the Number of Threads

In an ideal parallel system:

```text
S(p) = p
```

For example:

```text
1 thread  -> 1× speedup
2 threads -> 2× speedup
4 threads -> 4× speedup
10 threads -> 10× speedup
```

The actual measurements are different.

For the main `1500 × 1500` benchmark:

```text
10 threads -> 5.512× speedup
```

rather than the ideal:

```text
10 threads -> 10× speedup
```

Several factors explain this difference.

### Thread Management

Creating, scheduling, and joining threads requires additional work.

### CPU Core Limitations

The computer contains a finite number of physical CPU cores.

Creating additional software threads does not create additional hardware execution resources.

### Heterogeneous CPU Cores

The Apple M5 used for testing contains:

```text
4 Performance cores
6 Efficiency cores
```

These cores do not necessarily provide identical performance.

### Cache Behavior

Worker threads access large amounts of matrix data.

Large matrices cannot fit entirely into the fastest CPU caches.

This increases the cost of memory accesses.

### Memory Subsystem

Multiple CPU cores access memory simultaneously.

Memory bandwidth and the cache hierarchy are shared hardware resources.

### Operating System Scheduling

The operating system determines when and where individual threads execute.

The exact scheduling behavior can vary between program runs.

---

## 38. Parallel Efficiency Analysis

The measured efficiency for the `1500 × 1500` benchmark was:

```text
1 thread   -> 100.0%
2 threads  -> 101.2%
4 threads  -> 93.4%
6 threads  -> 75.4%
8 threads  -> 64.4%
10 threads -> 55.1%
12 threads -> 45.7%
16 threads -> 34.2%
```

Efficiency generally decreases as the number of worker threads increases.

This does not mean that additional workers are always useless.

For example:

```text
4 threads  -> 6.794006 s
10 threads -> 4.605120 s
```

Increasing the number of workers from 4 to 10 still reduced execution time significantly.

However, the number of workers increased by a factor of 2.5, while the execution time did not improve by the same factor.

This distinction is important:

```text
Execution time
    -> How long does the computation take?

Speedup
    -> How many times faster is the parallel version?

Efficiency
    -> How effectively are the available worker threads being used
       relative to ideal linear speedup?
```

---

## 39. Why Threads Are Suitable for This Task

Matrix multiplication is naturally suitable for parallel processing because different output elements can be calculated independently.

For example, calculating:

```text
C[0][0]
```

does not require the already calculated value of:

```text
C[1][0]
```

One worker can therefore calculate one group of rows while another worker calculates a different group.

The workers need access to the same input matrices.

Threads are convenient for this because all threads of the same process share the same address space.

There is no need to create separate copies of matrices `A` and `B` for every worker.

The selected row-based decomposition also prevents write conflicts because each output row belongs to exactly one worker.

---

## 40. Processes and Threads

A process owns resources such as:

- virtual address space;
- program code;
- global data;
- heap;
- open file descriptors.

Threads inside the same process share these process resources.

However, every thread has its own execution state, including:

- stack;
- CPU registers;
- instruction pointer;
- thread identifier.

In this program, all worker threads share matrices `A`, `B`, and `C`.

Each worker, however, has its own execution state and its own `TD` structure describing the assigned range of rows.

This shared-memory model is one of the main reasons threads are convenient for parallel matrix multiplication.

---

## 41. Resource Management

All matrices are dynamically allocated using `malloc()`.

After the computation is completed, the allocated memory is released using `free()`.

This includes:

- every row of matrix `A`;
- every row of matrix `B`;
- every row of matrix `C`;
- the arrays of row pointers;
- the array of `pthread_t` objects;
- the array of worker data structures.

Correct resource cleanup prevents memory leaks during normal program execution.

---

# Conclusions

The laboratory work demonstrated the practical use of operating system threads for parallel data processing.

A multithreaded complex matrix multiplication algorithm was implemented using POSIX Threads.

The computation was divided by rows of the resulting matrix. Each worker thread received its own non-overlapping interval of rows.

Because matrices `A` and `B` are only read during the computation and each worker writes to a separate region of matrix `C`, no mutex is required for matrix elements.

`pthread_join()` is used as the synchronization point between the main thread and the worker threads.

The experiments confirmed that multithreading can significantly reduce the execution time of computationally intensive matrix multiplication.

For `1500 × 1500` matrices:

```text
1 worker thread:
25.383677 s

10 worker threads:
4.605120 s
```

The ten-thread version was therefore approximately:

```text
5.51× faster
```

than the one-thread version.

However, the experiments also demonstrated that increasing the number of worker threads does not produce unlimited performance improvement.

For the same `1500 × 1500` matrices:

```text
10 threads -> 4.605120 s
12 threads -> 4.624499 s
16 threads -> 4.644194 s
```

The performance therefore reached a plateau at approximately 10 worker threads.

The test computer contains 10 physical CPU cores, so creating more CPU-bound worker threads does not provide additional physical computing resources.

The extreme test with 1000 worker threads further demonstrated this effect.

Its execution time was approximately:

```text
5.92 s
```

which was worse than the approximately `4.61 s` obtained with 10 worker threads.

This demonstrates the negative effects of excessive CPU oversubscription.

The input-size experiment also showed that the effectiveness of multithreading depends on the amount of useful computation.

For small `100 × 100` matrices, ten-thread efficiency was only:

```text
35.05%
```

while for `1000 × 1000` matrices it reached approximately:

```text
59.17%
```

For small workloads, thread creation, scheduling, and synchronization represent a significant fraction of total execution time.

As the workload becomes larger, these fixed costs become smaller relative to the amount of useful matrix computation.

The main conclusions are:

1. Matrix multiplication can be efficiently parallelized because different output rows can be calculated independently.
2. POSIX Threads allow multiple workers to operate on shared matrices within the same process.
3. A mutex is not required when workers only read shared input data and write to mutually exclusive regions of the output matrix.
4. `pthread_join()` provides the synchronization required to wait for all worker threads to complete.
5. Multithreading significantly reduced the execution time for large matrices.
6. Speedup is not generally equal to the number of worker threads because parallel execution introduces overhead and is limited by the available hardware.
7. Parallel efficiency generally decreases as the number of worker threads increases.
8. Increasing the number of CPU-bound threads beyond the available CPU resources does not necessarily improve performance.
9. Excessive thread creation can reduce performance because of scheduling, context switching, cache competition, and other shared-resource costs.
10. Larger computational workloads generally make thread-management overhead less significant relative to useful computation.
11. Performance measurements are not perfectly deterministic and can vary because of operating system scheduling, CPU behavior, cache state, and background system activity.

Overall, the laboratory work demonstrates both the advantages and the practical limitations of shared-memory parallelism using POSIX Threads.