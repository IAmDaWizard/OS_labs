# Operating Systems Labs

A collection of laboratory works for the **Operating Systems** course.

This repository contains practical assignments focused on operating system concepts, process management, inter-process communication, system calls, software diagnostics, multithreading, synchronization, and parallel computing.

The laboratory works include low-level programming assignments implemented primarily in **C** using POSIX interfaces, as well as practical operating system analysis using diagnostic tools.

## Repository Structure

Each laboratory work is stored in a separate directory and contains its own source files, supporting files, build configuration, and documentation when required.

```text
OS_labs/
├── lab_01/
│   ├── file/
│   ├── include/
│   ├── src/
│   ├── .gitignore
│   ├── CMakeLists.txt
│   └── README.md
│
├── lab_02/
│   ├── files/
│   └── README.md
│
├── lab_03/
│   ├── include/
│   ├── src/
│   ├── .gitignore
│   ├── CMakeLists.txt
│   └── README.md
│
├── .gitignore
├── CMakeLists.txt
├── LICENSE
└── README.md
```

Build directories and compiled executable files are not part of the source structure of the project and are excluded from version control where applicable.

The exact internal structure may differ between laboratory works depending on the assignment.

## Laboratory Works

| Lab | Topic | Status |
|---|---|---|
| [Lab 01](./lab_01/) | Processes and Pipes | Completed |
| [Lab 02](./lab_02/) | System Call Profiling | Completed |
| [Lab 03](./lab_03/) | Multithreaded Matrix Multiplication | Completed |
| Lab 04 | — | Planned |

The list will be updated as new laboratory works are completed.

---

## Lab 01 — Processes and Pipes

The first laboratory work focuses on **process management and inter-process communication** using POSIX system calls.

The program consists of separate parent and child processes.

The parent process reads the path to an input file, creates an unnamed pipe, starts a child process, redirects its standard input and output, and executes a separate child program.

The child process reads data from the redirected standard input, processes it, and sends the results back to the parent through the pipe.

The laboratory work covers:

- process creation using `fork()`;
- execution of another program using `execl()`;
- inter-process communication using an unnamed pipe;
- standard input and output redirection using `dup2()`;
- working with file descriptors;
- reading and writing data through POSIX interfaces;
- process synchronization using `waitpid()`;
- parent and child process interaction;
- process exit handling;
- system call error handling.

The laboratory demonstrates one of the fundamental differences between processes and threads: processes have separate virtual address spaces and require explicit inter-process communication mechanisms to exchange data.

More information about the implementation, process interaction, pipe communication, building, and running the program can be found in the [Lab 01 README](./lab_01/README.md).

---

## Lab 02 — System Call Profiling

The second laboratory work focuses on **analyzing the interaction between user-space software and the operating system kernel through system calls**.

The GNU nano text editor was selected as third-party software for analysis. Its execution was traced on Linux using `strace`.

During the experiment, a text file was opened in nano, modified, saved, and closed. The resulting system call trace was then analyzed to determine which operating system services were involved.

The laboratory work covers:

- tracing an application using `strace`;
- analyzing system call arguments and return values;
- grouping system calls according to their purpose;
- analyzing file operations;
- working with file descriptors;
- observing terminal input and output;
- analyzing signal handling;
- observing virtual memory operations;
- analyzing permission checks;
- tracing file reading and writing;
- identifying the relationship between application functionality and system calls.

Several important groups of system calls were observed during the experiment.

### File Operations

```text
openat()
read()
write()
close()
fstat()
newfstatat()
fsync()
```

### Terminal Interaction

```text
ioctl()
ppoll()
read()
write()
```

### Memory Management

```text
mmap()
munmap()
mprotect()
brk()
```

### Signal Handling

```text
rt_sigaction()
rt_sigprocmask()
```

### Process Management

```text
execve()
exit_group()
```

### Permission and User Operations

```text
faccessat()
getuid()
geteuid()
getgid()
getegid()
```

The trace demonstrates the general sequence of system calls used when reading a file:

```text
openat()
   |
   v
fstat()
   |
   v
read()
   |
   v
close()
```

A modified file is saved using operations such as:

```text
openat()
   |
   v
fstat()
   |
   v
write()
   |
   v
fsync()
   |
   v
close()
```

Files related to the experiment are stored in:

```text
lab_02/files/
```

More information about the experiment, system call groups, file descriptor usage, and trace analysis can be found in the [Lab 02 README](./lab_02/README.md).

---

## Lab 03 — Multithreaded Matrix Multiplication

The third laboratory work focuses on **operating system threads, shared-memory parallelism, synchronization, and parallel performance analysis** using POSIX Threads.

The task is to multiply two matrices containing complex numbers using multiple worker threads.

For matrices:

```text
A: m × n
B: n × p
```

the resulting matrix has dimensions:

```text
C: m × p
```

and each element is calculated as:

```text
C[i][j] = sum(A[i][k] * B[k][j])
```

### Multithreading

The maximum number of worker threads is specified using the `-t` command-line option:

```bash
./main -t 8
```

The program uses:

```c
pthread_create()
```

to create worker threads and:

```c
pthread_join()
```

to wait for their completion.

The rows of the resulting matrix are divided between worker threads.

Each worker receives its own non-overlapping interval:

```text
[start_row, end_row)
```

and calculates only the corresponding rows of matrix `C`.

Matrices `A` and `B` are shared between all workers and are read-only during the computation.

Matrix `C` is also shared, but every worker writes to a different range of rows.

Because there are no conflicting writes to the same matrix elements, a mutex is not required for the matrix multiplication itself.

### Input Modes

The program supports automatic matrix generation:

```bash
./main -t 8
```

and manual matrix input:

```bash
./main -t 8 -m
```

Automatic generation is used for large performance experiments, while manual input can be used to verify the correctness of matrix multiplication.

### Performance Measurement

Execution time is measured using:

```c
clock_gettime(CLOCK_MONOTONIC, ...)
```

The measured interval starts immediately before worker thread creation and ends after all workers have completed.

Therefore, the measurement includes:

```text
thread creation
       |
       v
parallel matrix multiplication
       |
       v
pthread_join()
       |
       v
timer stop
```

Matrix allocation, input generation, manual input, and result printing are excluded from the measured interval.

### Speedup and Efficiency

Parallel performance is evaluated using two metrics.

Speedup:

```text
S(p) = T(1) / T(p)
```

where:

```text
T(1) = execution time with one worker
T(p) = execution time with p workers
```

Parallel efficiency:

```text
E(p) = S(p) / p
```

Efficiency can also be expressed as a percentage:

```text
E(p) = S(p) / p × 100%
```

### Thread Scaling Experiment

The main benchmark used two `1500 × 1500` matrices.

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

The program demonstrates strong performance improvement for relatively small thread counts.

For example:

```text
1 worker  -> 25.383677 s
10 workers -> 4.605120 s
```

which corresponds to approximately:

```text
5.51× speedup
```

After approximately 10 workers, performance reaches a plateau:

```text
10 workers -> 4.605120 s
12 workers -> 4.624499 s
16 workers -> 4.644194 s
```

Adding more CPU-bound threads therefore does not automatically improve performance.

### Input Size Experiment

The influence of the input data size was also investigated.

| Matrix Size | 1 Thread | 10 Threads | Speedup | Efficiency |
|---:|---:|---:|---:|---:|
| 100 × 100 | 0.013432 s | 0.003832 s | 3.505× | 35.05% |
| 500 × 500 | 0.562025 s | 0.113521 s | 4.951× | 49.51% |
| 1000 × 1000 | 5.875144 s | 0.992891 s | 5.917× | 59.17% |
| 1500 × 1500 | 25.383677 s | 4.605120 s | 5.512× | 55.12% |

Small workloads are affected more strongly by thread creation and synchronization overhead.

As the matrix size increases, the amount of useful computation grows approximately as:

```text
O(N³)
```

making the fixed thread-management overhead relatively less significant.

### CPU Oversubscription

An additional experiment was performed with 1000 requested worker threads for `1500 × 1500` matrices.

The execution time was approximately:

```text
5.92 s
```

compared with approximately:

```text
4.61 s
```

using 10 worker threads.

This demonstrates CPU oversubscription.

Creating more software threads does not create additional physical CPU cores. Excessive numbers of CPU-bound threads can instead increase scheduling, context-switching, and thread-management overhead.

More information about the implementation, synchronization, thread distribution, correctness testing, benchmarks, speedup, efficiency, and experimental results can be found in the [Lab 03 README](./lab_03/README.md).

---

## Technologies and Tools

The laboratory works use:

- **C**
- **POSIX API**
- **POSIX Threads (`pthread`)**
- **Linux**
- **macOS**
- **GCC / Clang**
- **CMake**
- **strace**
- **Git**
- Unix/POSIX process and thread diagnostic tools

Linux is used when Linux-specific tools such as `strace` are required.

POSIX programming assignments are also developed and tested on macOS where applicable.

---

## Building the Project

The repository contains a root `CMakeLists.txt` as well as individual CMake configurations for programming laboratory works.

A standard CMake build can be performed using:

```bash
cmake -S . -B build
cmake --build build
```

Individual programs can also be compiled directly using GCC or Clang.

For example:

```bash
gcc -Wall -Wextra -Wpedantic source.c -o program
```

Programs using POSIX Threads can be compiled with:

```bash
gcc src/*.c -o main -pthread
```

Compiler warning flags used during development include:

```text
-Wall
-Wextra
-Wpedantic
```

These options help detect common programming errors and non-portable constructs.

Analysis-oriented laboratory works such as Lab 02 do not necessarily require compilation and instead use operating system diagnostic tools.

---

## Goals

The main goals of this repository are to:

- understand fundamental operating system concepts;
- gain practical experience with POSIX interfaces;
- understand the interaction between user-space applications and the operating system kernel;
- learn how processes are created and executed;
- study inter-process communication mechanisms;
- understand pipes and file descriptors;
- work with low-level input and output;
- analyze system calls performed by real applications;
- understand the difference between processes and threads;
- create and manage POSIX threads;
- understand shared-memory parallelism;
- understand thread synchronization;
- identify and prevent data races;
- understand when synchronization mechanisms such as mutexes are required;
- distribute computational work between multiple threads;
- measure parallel program performance;
- analyze speedup and parallel efficiency;
- understand thread-management overhead;
- investigate CPU oversubscription;
- gain experience with operating system diagnostic tools;
- develop reliable error handling;
- improve low-level programming, debugging, and performance-analysis skills.

---

## License

This project is distributed under the terms of the [MIT License](./LICENSE).