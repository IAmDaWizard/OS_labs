# Operating Systems Labs

A collection of laboratory works for the **Operating Systems** course.

This repository contains practical assignments focused on operating system concepts, process management, inter-process communication, memory management, synchronization, and other low-level programming topics.

The laboratory works are implemented primarily in **C** using POSIX system calls and standard C library facilities.

## Repository Structure

Each laboratory work is stored in a separate directory and contains its own source code, headers, test data, build configuration, and documentation.

```text
OS_labs/
├── lab_01/
│   ├── file/
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
│
├── ...
│
├── CMakeLists.txt
├── LICENSE
└── README.md
```

The exact structure may differ between laboratory works depending on the task.

## Laboratory Works

| Lab | Topic | Status |
|---|---|---|
| [Lab 01](./lab_01/) | Processes and Pipes | Completed |
| Lab 02 | — | Planned |
| Lab 03 | — | Planned |
| Lab 04 | — | Planned |

The list will be updated as new laboratory works are completed.

## Lab 01 — Processes and Pipes

The first laboratory work focuses on **process management and inter-process communication**.

It demonstrates:

- process creation using `fork()`;
- execution of a separate program using `execl()`;
- communication between processes using an unnamed pipe;
- standard input and output redirection using `dup2()`;
- working with file descriptors;
- reading and writing data using `read()` and `write()`;
- process synchronization using `waitpid()`;
- child process exit status handling;
- system call error handling.

More information about the implementation, data flow, building, and running the program can be found in the [Lab 01 README](./lab_01/README.md).

## Technologies

The laboratory works use:

- **C**
- **POSIX API**
- **GCC / Clang**
- **CMake**
- **Git**

Development is performed on a Unix-like environment.

## Compilation

Individual laboratory works may provide their own compilation instructions.

For example, C programs can be compiled directly using GCC:

```bash
gcc -Wall -Wextra -Wpedantic source.c -o program
```

The warning flags are used to improve code quality during development:

- `-Wall` — enables common compiler warnings;
- `-Wextra` — enables additional warnings;
- `-Wpedantic` — warns about code that does not strictly follow the selected C standard.

Some laboratory works also provide a `CMakeLists.txt` and can be built using CMake.

## Goals

The main goals of this repository are to:

- understand fundamental operating system concepts;
- gain practical experience with POSIX system calls;
- learn how processes interact with the operating system;
- understand process creation and execution;
- study inter-process communication mechanisms;
- practice working with low-level I/O and file descriptors;
- develop reliable error handling for system calls;
- improve practical C programming skills.

## License

This project is distributed under the terms of the [MIT License](./LICENSE).