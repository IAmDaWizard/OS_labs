# Lab 04 — Processes, Memory-Mapped Files, and Semaphores

Laboratory work on process management and inter-process communication in C using POSIX system calls, memory-mapped files, and named semaphores.

The program consists of two separate programs: a **parent process** and a **child process**. The parent creates the child, redirects its standard input to a selected file, and starts the child program using `exec`. The processes exchange messages through a shared memory region created with `mmap()`.

Named POSIX semaphores synchronize access to the shared memory and prevent one process from reading or overwriting data at the wrong time.

## Task — Variant 6

The parent process reads the name of an input file from standard input and opens it for reading.

The input file contains lines in the following format:

```text
number number number ...
```

Each line may contain an arbitrary number of integer values of type `int`.

The child process reads the file line by line, calculates the sum of all numbers in each line, and places each result in shared memory.

The parent process receives the results from shared memory and prints them to its standard output.

The parent and child processes are implemented as separate programs. Only the parent program is started manually. The parent automatically creates and starts the child program using `fork()` and `execlp()`.

## Process Communication

The program uses the following data flow:

```text
Input file
    │
    │ dup2()
    ▼
Child stdin
    │
    │ getline()
    ▼
Split the line into values
    │
    │ strtok() and strtol()
    ▼
Calculate the sum
    │
    │ wait for the empty semaphore
    ▼
Write a message to SharedData
    │
    │ post the ready semaphore
    ▼
Parent reads the shared memory
    │
    │ printf()
    ▼
Terminal
```

The temporary file used for shared memory is created by the parent with `mkstemp()`. Its size is set to `sizeof(SharedData)` using `ftruncate()`, after which both processes map it with:

```c
mmap(..., PROT_READ | PROT_WRITE, MAP_SHARED, ...);
```

The `MAP_SHARED` flag makes modifications visible to both processes.

## Shared Data

The processes exchange one message at a time through the following structure:

```c
typedef struct {
    uint32_t magic;
    MessageType type;
    size_t line_number;
    int64_t sum;
    char error_text[ERROR_TEXT_SIZE];
} SharedData;
```

The structure contains:

| Field | Purpose |
|---|---|
| `magic` | Control value used to identify the expected shared-memory format |
| `type` | Message type: result, completion, or error |
| `line_number` | Number of the processed input line |
| `sum` | Calculated sum of the integer values |
| `error_text` | Description of an error detected by the child |

The following message types are used:

| Message | Meaning |
|---|---|
| `MESSAGE_RESULT` | A line was processed and its sum is available |
| `MESSAGE_DONE` | The entire input file was processed |
| `MESSAGE_ERROR` | The child detected invalid input or a read error |

## Semaphore Synchronization

The shared structure is a single message slot. Two named POSIX semaphores synchronize access to it:

| Semaphore | Initial value | Meaning |
|---|---:|---|
| `empty` | `1` | The shared slot is free and the child may write a message |
| `ready` | `0` | A message is available and the parent may read it |

The child sends a message using the following sequence:

1. Waits for `empty` using `sem_wait()`.
2. Writes the message to `SharedData`.
3. Signals `ready` using `sem_post()`.

The parent receives a message using the following sequence:

1. Waits for `ready`.
2. Reads and processes the message from `SharedData`.
3. Signals `empty` so the child may write the next message.

The synchronization cycle is:

```text
Child                                 Parent
  │                                      │
  ├─ sem_wait(empty)                     │
  ├─ write SharedData                    │
  ├─ sem_post(ready) ───────────────────►│
  │                                      ├─ read SharedData
  │◄────────────────── sem_post(empty) ──┤
  │                                      │
  └─ process the next line               └─ wait for the next message
```

Without `empty`, the child could overwrite a result before the parent reads it. Without `ready`, the parent could read the shared structure before the child finishes writing it.

## Parent Process

The parent process performs the following steps:

1. Reads the input file name from standard input.
2. Opens the input file using `open()`.
3. Creates a unique temporary file using `mkstemp()`.
4. Sets the temporary file size using `ftruncate()`.
5. Maps the temporary file into memory using `mmap()` and `MAP_SHARED`.
6. Creates the named semaphores `empty` and `ready`.
7. Creates a child process using `fork()`.
8. Redirects the child's standard input to the input file using `dup2()`.
9. Starts the separate child program using `execlp()`.
10. Waits for messages from the child.
11. Prints every received sum to standard output.
12. Waits for the child process using `waitpid()` and checks its exit status.
13. Closes and removes all semaphores, mappings, file descriptors, and the temporary file.

## Child Process

The child process performs the following steps:

1. Receives the mapping file descriptor and semaphore names through command-line arguments.
2. Maps the same temporary file using `mmap()` and `MAP_SHARED`.
3. Opens the existing named semaphores using `sem_open()`.
4. Reads the redirected standard input line by line using `getline()`.
5. Splits each line into separate values using `strtok()`.
6. Converts every value using `strtol()`.
7. Checks that every value is within the range of type `int`.
8. Calculates the sum using `int64_t`.
9. Sends a `MESSAGE_RESULT` message for every successfully processed line.
10. Sends `MESSAGE_ERROR` if invalid data or a read error is detected.
11. Sends `MESSAGE_DONE` after reaching the end of the file.
12. Closes its semaphores, mapping, and file descriptor before termination.

## Project Structure

```text
lab_04/
├── file/
│   ├── empty.txt
│   └── input.txt
├── include/
│   ├── child.h
│   ├── common.h
│   └── parent.h
├── src/
│   ├── child.c
│   └── parent.c
├── .gitignore
├── CMakeLists.txt
└── README.md
```

## CMake Configuration

The `CMakeLists.txt` file creates two executable targets:

```cmake
add_executable(parent
        src/parent.c
        include/parent.h
        include/common.h
)

add_executable(child
        src/child.c
        include/child.h
        include/common.h
)

target_include_directories(parent PRIVATE include)
target_include_directories(child PRIVATE include)
```

Both executables are required, but only `parent` is started manually. The `child` executable must be located in the same directory because `parent` starts it automatically.

## Build with CMake

From the `lab_04` directory, run:

```bash
cmake -S . -B build
cmake --build build
```

This creates two executables:

```text
build/parent
build/child
```

If `lab_04` is included in a larger CMake project with `add_subdirectory(lab_04)`, the executables are normally placed in:

```text
cmake-build-debug/lab_04/parent
cmake-build-debug/lab_04/child
```

## Build with GCC

The programs can also be compiled manually:

```bash
gcc -std=c17 -Wall -Wextra -Wpedantic -Iinclude src/parent.c -o parent -pthread
gcc -std=c17 -Wall -Wextra -Wpedantic -Iinclude src/child.c -o child -pthread
```

On older Linux systems, POSIX real-time functions may additionally require `-lrt`.

## Run

Run only the parent program:

```bash
./build/parent
```

The parent automatically creates and starts the child process. The child executable should not be started manually.

After the prompt, enter the input file path:

```text
file/input.txt
```

For example, for the following input:

```text
10 20 30
-5 12 8 100
1 2 3 4 5 6
```

the program produces:

```text
Строка 1: сумма = 60
Строка 2: сумма = 115
Строка 3: сумма = 21
```

## System Calls and Functions

The implementation uses the following POSIX system calls and standard C library functions:

| Function | Purpose |
|---|---|
| `open()` | Opens the input file |
| `mkstemp()` | Creates a unique temporary file for shared memory |
| `ftruncate()` | Sets the size of the mapping file |
| `mmap()` | Maps a file into the process address space |
| `munmap()` | Removes a memory mapping |
| `sem_open()` | Creates or opens a named semaphore |
| `sem_wait()` | Waits until the shared slot is available |
| `sem_trywait()` | Checks whether a child message is ready |
| `sem_post()` | Signals that the shared slot or message is available |
| `sem_close()` | Closes a semaphore in the current process |
| `sem_unlink()` | Removes a named semaphore from the system |
| `fork()` | Creates the child process |
| `dup2()` | Redirects the child's standard input |
| `execlp()` | Replaces the child process image with the child program |
| `waitpid()` | Checks or waits for child termination |
| `kill()` | Terminates the child after a parent-side error |
| `close()` | Closes a file descriptor |
| `unlink()` | Removes the temporary mapping file |
| `getline()` | Reads input line by line |
| `strtok()` | Splits a line into separate values |
| `strtol()` | Converts text to an integer value |
| `perror()` | Reports system-call errors |

## Error Handling

The program checks failures of the main system calls and library operations, including:

- opening the input file;
- creating and resizing the temporary file;
- creating the memory mapping;
- creating and opening semaphores;
- creating the child process;
- redirecting standard input;
- starting the child executable;
- waiting for semaphore messages;
- reading the input file;
- parsing integer values;
- waiting for child termination.

The parent uses `waitpid()` and the following macros to verify the child process result:

- `WIFEXITED()` checks whether the child terminated normally;
- `WEXITSTATUS()` obtains the child's exit code.

An exit code of `0` indicates successful execution.

The cleanup functions close all resources even when an error occurs. The parent is responsible for unlinking the named semaphores and deleting the temporary mapping file.

## Conclusion

During this laboratory work, process creation and inter-process communication through memory-mapped files were studied and implemented.

The program demonstrates how a parent process can create a child process using `fork()`, redirect its standard input using `dup2()`, and replace the child process image using `execlp()`.

Unlike communication through an unnamed pipe, the parent and child exchange structured messages through a file-backed shared memory region created with `mmap()` and `MAP_SHARED`.

Named POSIX semaphores implement a one-slot producer-consumer synchronization scheme. The child produces result messages, while the parent consumes and prints them. The `empty` semaphore prevents the child from overwriting unread data, and the `ready` semaphore prevents the parent from reading incomplete data.

As a result, practical experience was gained with process management, standard-input redirection, file descriptors, memory mapping, process synchronization, structured inter-process messages, child exit-status handling, and system-resource cleanup.

## Requirements

- C17-compatible C compiler
- POSIX-compatible operating system
- CMake or GCC/Clang
- POSIX memory mapping and named semaphore support
