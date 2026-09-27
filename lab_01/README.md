# Lab 01 — Processes and Pipes

Laboratory work on process management and inter-process communication in C using POSIX system calls.

The program consists of two separate processes: a **parent process** and a **child process**. The parent creates the child, redirects its standard input and output, and communicates with it through an unnamed pipe.

## Task

The parent process reads the name of an input file from standard input and opens it for reading.

The input file contains lines in the following format:

```text
number number number ...
```

Each line may contain an arbitrary number of integer values.

The child process reads the file line by line, calculates the sum of all numbers in each line, and writes the result to its standard output.

The parent process receives the results from the child through a pipe and prints them to its own standard output.

## Process Communication

The program uses the following data flow:

```text
Input file
    │
    │
    ▼
Child stdin
    │
    │ getline()
    ▼
Parse numbers
    │
    │ strtol()
    ▼
Calculate sum
    │
    │ fprintf(stdout)
    ▼
Pipe
    │
    │ read()
    ▼
Parent
    │
    │ write()
    ▼
Terminal
```

The parent process performs the following steps:

1. Reads the input file name.
2. Opens the file using `open()`.
3. Creates an unnamed pipe using `pipe()`.
4. Creates a child process using `fork()`.
5. Redirects the child's standard input to the input file using `dup2()`.
6. Redirects the child's standard output to the pipe using `dup2()`.
7. Starts the child program using `execl()`.
8. Reads the results from the pipe.
9. Writes the received data to standard output.
10. Waits for the child process using `waitpid()` and checks its exit status.

The child process:

1. Reads input line by line from `stdin`.
2. Parses integer values using `strtol()`.
3. Calculates the sum of the numbers in each line.
4. Writes each result to `stdout`.

Because the child's `stdin` and `stdout` are redirected by the parent, the child does not need to know that its input comes from a file and its output is sent through a pipe.

## Project Structure

```text
lab_01/
├── file/
│   └── input.txt
├── include/
│   ├── child.h
│   └── parent.h
├── src/
│   ├── child.c
│   └── parent.c
├── CMakeLists.txt
└── README.md
```

## Build

The programs can be compiled using GCC:

```bash
gcc -Wall -Wextra -Wpedantic src/parent.c -o parent
gcc -Wall -Wextra -Wpedantic src/child.c -o child
```

## Run

Run the parent process:

```bash
./parent
```

Then enter the path to the input file:

```text
file/input.txt
```

For example, for the following input:

```text
1 2 3
10 20 30 40
5 -2 8
100
-10 -20 -30
```

the program produces:

```text
6
100
11
100
-60
```

## System Calls and Functions

The implementation uses several POSIX system calls and standard C library functions:

| Function | Purpose |
|---|---|
| `open()` | Opens the input file |
| `pipe()` | Creates an unnamed pipe |
| `fork()` | Creates the child process |
| `dup2()` | Redirects standard input and output |
| `execl()` | Replaces the child process image with the child program |
| `read()` | Reads data from the pipe |
| `write()` | Writes received data to standard output |
| `waitpid()` | Waits for the child process to terminate |
| `close()` | Closes unused file descriptors |
| `getline()` | Reads input line by line |
| `strtol()` | Converts numbers from text to integer values |
| `perror()` | Reports system call errors |

## Error Handling

System call failures are checked and reported using `perror()`.

The parent also checks the termination status of the child process using:

- `WIFEXITED()` to determine whether the child terminated normally;
- `WEXITSTATUS()` to obtain the child's exit code.

An exit code of `0` indicates successful execution.

## Conclusion

During this laboratory work, process creation and inter-process communication in a POSIX environment were studied and implemented.

The program demonstrates how a parent process can create a child process using `fork()`, configure its standard input and output using `dup2()`, and replace its program image using `execl()`.

An unnamed pipe is used to transfer data from the child process to the parent process. The child reads data from the redirected standard input, processes each line, and writes the calculated results to its standard output. The parent receives these results through the pipe and prints them to the terminal.

The laboratory work also provided practical experience with file descriptors, process synchronization using `waitpid()`, child process exit status handling, and system call error handling.

As a result, the basic principles of process management, standard stream redirection, and inter-process communication using pipes were studied in practice.

## Requirements

- C
- POSIX-compatible operating system
- GCC or another compatible C compiler