# Operating Systems Labs

A collection of laboratory works for the **Operating Systems** course.

This repository contains practical assignments focused on operating system concepts, process management, inter-process communication, system calls, software diagnostics, memory management, synchronization, and other low-level topics.

The laboratory works include both low-level programming assignments implemented primarily in **C** using the POSIX API and practical operating system analysis using Linux diagnostic tools.

## Repository Structure

Each laboratory work is stored in a separate directory and contains the files required for the corresponding assignment.

```text
OS_labs/
├── lab_01/
│   ├── file/
│   ├── include/
│   ├── src/
│   ├── CMakeLists.txt
│   └── README.md
│
├── lab_02/
│   ├── files/
│   │   └── nano_trace.txt
│   │   └── test.txt
│   └── README.md
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
| [Lab 02](./lab_02/) | System Call Profiling | Completed |
| Lab 03 | — | Planned |
| Lab 04 | — | Planned |

The list will be updated as new laboratory works are completed.

## Lab 01 — Processes and Pipes

The first laboratory work focuses on **process management and inter-process communication** using POSIX system calls.

The program consists of separate parent and child processes and demonstrates communication between them through an unnamed pipe.

It covers:

- process creation using `fork()`;
- execution of a separate program using `execl()`;
- communication between processes using an unnamed pipe;
- standard input and output redirection using `dup2()`;
- working with file descriptors;
- reading and writing data using `read()` and `write()`;
- process synchronization using `waitpid()`;
- child process exit status handling;
- system call error handling.

More information about the implementation, process interaction, data flow, building, and running the program can be found in the [Lab 01 README](./lab_01/README.md).

## Lab 02 — System Call Profiling

The second laboratory work focuses on **analyzing the interaction between user-space software and the operating system kernel through system calls**.

The GNU nano text editor was selected as the third-party software for analysis. Its execution was traced on Linux using `strace`.

During the experiment, a text file was opened in nano, modified, saved, and closed. The resulting system call trace was analyzed to determine which operating system services were used to implement this functionality.

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

The experiment demonstrates several important system call groups:

- file operations — `openat()`, `read()`, `write()`, `close()`, `fstat()`, `newfstatat()`, `fsync()`;
- terminal interaction — `ioctl()`, `ppoll()`, `read()`, `write()`;
- memory management — `mmap()`, `munmap()`, `mprotect()`, `brk()`;
- signal handling — `rt_sigaction()`, `rt_sigprocmask()`;
- process management — `execve()`, `exit_group()`;
- permission and user operations — `faccessat()`, `getuid()`, `geteuid()`, `getgid()`, `getegid()`.

The trace also demonstrates the system call sequence used when reading a file:

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

and when saving a modified file:

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

The complete execution trace is stored in:

```text
lab_02/traces/nano_trace.txt
```

More information about the experiment, system call groups, file descriptor usage, and trace analysis can be found in the [Lab 02 README](./lab_02/README.md).

## Technologies and Tools

The laboratory works use:

- **C**
- **POSIX API**
- **Linux**
- **GCC / Clang**
- **CMake**
- **strace**
- **Git**

The programming assignments are developed using standard Unix/POSIX interfaces, while Linux is used when Linux-specific diagnostic tools or system call analysis are required.

## Compilation

Programming laboratory works provide their own compilation instructions when applicable.

For example, C programs can be compiled directly using GCC:

```bash
gcc -Wall -Wextra -Wpedantic source.c -o program
```

The warning flags are used to improve code quality during development:

- `-Wall` — enables common compiler warnings;
- `-Wextra` — enables additional warnings;
- `-Wpedantic` — warns about code that does not strictly follow the selected C standard.

Some laboratory works also provide a `CMakeLists.txt` and can be built using CMake.

Analysis-oriented laboratory works, such as Lab 02, may not require compilation and instead use operating system diagnostic tools.

## Goals

The main goals of this repository are to:

- understand fundamental operating system concepts;
- gain practical experience with POSIX system calls;
- understand the interaction between user-space applications and the operating system kernel;
- learn how processes are created and executed;
- study inter-process communication mechanisms;
- understand file descriptors and low-level I/O;
- analyze real system calls performed by applications;
- study process, file, memory, terminal, and signal-related operating system mechanisms;
- gain experience with operating system diagnostic tools;
- develop reliable error handling for system calls;
- improve practical low-level programming and debugging skills.

## License

This project is distributed under the terms of the [MIT License](./LICENSE).