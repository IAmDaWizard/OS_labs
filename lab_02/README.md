# Lab 02 — System Call Profiling

## Overview

This laboratory work focuses on analyzing the interaction between user-space software and the Linux kernel through system calls.

The GNU nano text editor was selected as the third-party software for analysis. Its execution was traced using `strace`.

During the experiment, an existing text file was opened in nano, modified, saved, and closed. The resulting trace was analyzed to identify system calls and group them according to the functionality they provide.

---

## Environment

- Operating System: Ubuntu 24.04.5 LTS ARM64
- Virtual Machine: UTM
- Architecture: ARM64
- Analyzed Software: GNU nano 7.2
- Diagnostic Tool: `strace`

---

## What Is a System Call?

A system call is a mechanism that allows a user-space program to request a service from the operating system kernel.

Applications normally cannot directly perform privileged operations such as accessing files, managing virtual memory, or controlling devices. Instead, they request these operations from the kernel through system calls.

A simplified interaction can be represented as:

```text
User-space application
        |
        | system call
        v
   Linux kernel
        |
        v
System resources
```

For example, when nano needs to read a file, the operation eventually results in system calls such as `openat()`, `read()`, and `close()`.

---

## Diagnostic Tool

`strace` was used to trace the system calls performed by nano.

The main tracing command was:

```bash
strace -o nano_trace.txt nano test.txt
```

The `-o` option redirects the trace produced by `strace` to the specified file.

The resulting trace contained 1121 lines.

---

## Experiment

First, a test file was created:

```bash
echo "Hello, Operating System!" > test.txt
```

Then nano was started under `strace`:

```bash
strace -o nano_trace.txt nano test.txt
```

During the execution:

1. nano was started;
2. `test.txt` was opened;
3. the contents of the file were read;
4. additional text was entered;
5. the file was saved;
6. nano was closed.

The trace was then analyzed to determine which system calls were associated with these operations.

---

## System Call Statistics

The system calls found in the trace were counted using:

```bash
awk '{print $1}' nano_trace.txt | sed 's/(.*//' | sort | uniq -c | sort -nr
```

The result was:

```text
163 read
129 rt_sigaction
128 ppoll
123 write
95  faccessat
86  openat
75  fstat
75  close
66  newfstatat
60  ioctl
27  mmap
11  brk
8   mprotect
7   munmap
6   rt_sigprocmask
6   geteuid
6   fcntl
5   getcwd
4   readlinkat
3   unlinkat
3   mkdirat
3   getuid
3   getgid
3   getegid
2   uname
2   socket
2   setfsuid
2   setfsgid
2   lseek
2   getpid
2   getdents64
2   connect
1   set_tid_address
1   set_robust_list
1   rseq
1   prlimit64
1   getrandom
1   futex
1   fsync
1   exit_group
1   execve
```

---

## System Call Groups

The system calls can be divided into several groups according to their purpose.

### 1. File Operations

Examples:

- `openat`
- `read`
- `write`
- `close`
- `fstat`
- `newfstatat`
- `faccessat`
- `lseek`
- `fsync`
- `unlinkat`
- `readlinkat`
- `getdents64`
- `fcntl`

These system calls were used for operations such as:

- opening files;
- reading file contents;
- writing modified data;
- obtaining file metadata;
- checking access permissions;
- synchronizing modified data;
- closing files;
- managing temporary files.

---

### 2. Memory Management

Examples:

- `mmap`
- `munmap`
- `mprotect`
- `brk`

These calls are related to the virtual memory of the process.

They are used for operations such as:

- allocating memory;
- mapping files and libraries into virtual memory;
- changing memory protection;
- releasing memory mappings.

---

### 3. Terminal and Input/Output Operations

Examples:

- `read`
- `write`
- `ioctl`
- `ppoll`

nano is an interactive terminal application, so a significant part of its execution consists of interacting with the terminal.

`ioctl()` was used to obtain and modify terminal settings.

For example:

```text
ioctl(0, TCGETS, ...)
ioctl(0, TCSETS, ...)
```

`ppoll()` was used to monitor file descriptors for events.

For example:

```text
ppoll([{fd=0, events=POLLIN}], ...)
```

Here, file descriptor `0` represents standard input. `POLLIN` indicates that the application is interested in data becoming available for reading.

---

### 4. Signal Handling

Examples:

- `rt_sigaction`
- `rt_sigprocmask`

These system calls were used to configure signal handlers and control which signals were temporarily blocked or unblocked.

This functionality is important for an interactive terminal application such as nano.

---

### 5. Process Management

Examples:

- `execve`
- `exit_group`
- `getpid`
- `prlimit64`

The beginning of the trace contains:

```text
execve("/usr/bin/nano", ["nano", "test.txt"], ...) = 0
```

`execve()` loads and executes the nano program.

At the end of the execution, `exit_group()` is used to terminate the process.

---

### 6. User and Permission Operations

Examples:

- `getuid`
- `geteuid`
- `getgid`
- `getegid`
- `setfsuid`
- `setfsgid`
- `faccessat`

These calls are related to user and group identifiers and access permissions.

For example:

```text
faccessat(AT_FDCWD, "test.txt", W_OK) = 0
```

This checks whether the process has permission to write to `test.txt`.

The return value `0` indicates that the check succeeded.

---

### 7. Runtime and Synchronization Operations

Examples:

- `futex`
- `set_tid_address`
- `set_robust_list`
- `rseq`

These are low-level system calls used by the runtime environment and Linux synchronization mechanisms.

They are not directly related to editing the text file but appear as part of normal process execution.

---

## File Descriptor Usage

Linux represents open resources using integer file descriptors.

The standard descriptors are:

```text
0 — standard input
1 — standard output
2 — standard error
```

When nano opened `test.txt`, the kernel returned file descriptor `3`:

```text
openat(AT_FDCWD, "/home/ubuntu/Desktop/test.txt", O_RDONLY) = 3
```

Subsequent operations could therefore refer to the file using descriptor `3` instead of its path.

After:

```text
close(3) = 0
```

the descriptor became available again and could be reused for another file.

---

## Reading the File

The trace clearly shows how nano opened and read `test.txt`.

First, the file was opened:

```text
openat(AT_FDCWD, "/home/ubuntu/Desktop/test.txt", O_RDONLY) = 3
```

`O_RDONLY` means that the file was opened in read-only mode.

The kernel returned file descriptor `3`.

nano then obtained information about the file:

```text
fstat(3, {st_mode=S_IFREG|0664, st_size=25, ...}) = 0
```

The file was a regular file with a size of 25 bytes.

The contents were then read:

```text
read(3, "Hello, Operating System!\n", 4096) = 25
```

nano requested up to 4096 bytes and received 25 bytes.

The next call returned:

```text
read(3, "", 4096) = 0
```

A return value of `0` from `read()` indicates end-of-file (EOF).

Finally, the descriptor was closed:

```text
close(3) = 0
```

Therefore, the file-reading sequence can be represented as:

```text
openat("test.txt", O_RDONLY)
        |
        v
      FD 3
        |
        v
    fstat(3)
        |
        v
     read(3)
        |
        v
     read(3)
      = 0
      (EOF)
        |
        v
    close(3)
```

---

## Saving the File

The trace also shows the system calls used when the modified file was saved.

The file was opened for writing:

```text
openat(
    AT_FDCWD,
    "test.txt",
    O_WRONLY|O_CREAT|O_TRUNC,
    0666
) = 3
```

The flags have the following meanings:

- `O_WRONLY` — open the file for writing;
- `O_CREAT` — create the file if it does not exist;
- `O_TRUNC` — truncate the existing file to zero length.

Immediately after opening the file, its size was zero:

```text
fstat(3, {st_mode=S_IFREG|0664, st_size=0, ...}) = 0
```

nano then wrote the modified contents:

```text
write(3, "Hello, Operating System!\nThe Wor"..., 44) = 44
```

The program requested to write 44 bytes, and the return value shows that all 44 bytes were successfully written.

The file was then synchronized:

```text
fsync(3) = 0
```

`fsync()` requests synchronization of modified file data with the underlying storage.

The file descriptor was then closed:

```text
close(3) = 0
```

After saving, nano checked the file again:

```text
newfstatat(
    AT_FDCWD,
    "test.txt",
    {st_mode=S_IFREG|0664, st_size=44, ...},
    0
) = 0
```

The size had changed from 25 bytes to 44 bytes.

The complete saving sequence can therefore be represented as:

```text
openat(
    "test.txt",
    O_WRONLY | O_CREAT | O_TRUNC
)
        |
        v
      FD 3
        |
        v
    fstat(3)
   st_size = 0
        |
        v
    write(3)
    44 bytes
        |
        v
     fsync(3)
        |
        v
     close(3)
        |
        v
   newfstatat()
  st_size = 44
```

---

## Terminal Output vs File Output

The trace demonstrates how the same `write()` system call can operate on different resources depending on the file descriptor.

During saving, nano displayed the following message in the terminal:

```text
write(1, "...[ Writing... ]...", 32) = 32
```

Here, file descriptor `1` represents standard output, so the data was written to the terminal.

The actual file contents were written using:

```text
write(3, "Hello, Operating System!\nThe Wor"..., 44) = 44
```

Here, descriptor `3` represented `test.txt`.

Therefore:

```text
write(1, ...) → terminal output
write(3, ...) → test.txt
```

This demonstrates that system calls operate on file descriptors rather than directly on file names after a resource has been opened.

---

## Temporary Swap File

nano also created a temporary swap file:

```text
./.test.txt.swp
```

Initially, nano checked whether the file existed:

```text
newfstatat(AT_FDCWD, "./.test.txt.swp", ...) = -1 ENOENT
```

`ENOENT` means that the requested file or directory did not exist.

nano then created the swap file:

```text
openat(
    AT_FDCWD,
    "./.test.txt.swp",
    O_WRONLY|O_CREAT|O_EXCL,
    0666
) = 3
```

Later, the temporary file was removed:

```text
unlinkat(AT_FDCWD, "./.test.txt.swp", 0) = 0
```

This also demonstrates that a failed system call does not necessarily indicate a program failure. Applications may intentionally check whether a resource exists and handle a negative result normally.

---

## Additional File Access

After `test.txt` was closed, file descriptor `3` was reused.

For example, nano later opened its configuration file:

```text
openat(
    AT_FDCWD,
    "/usr/share/nano/default.nanorc",
    O_RDONLY
) = 3
```

It was then read:

```text
read(3, "## This is an example of a defau"..., 1024) = 775
read(3, "", 1024) = 0
close(3) = 0
```

This demonstrates that a file descriptor is not permanently associated with a particular file.

After a descriptor is closed, its number may be reused for another resource.

---

## Functional System Call Profile

The observed behavior of nano can be summarized as follows:

| Software functionality | Main system calls |
|---|---|
| Program startup | `execve` |
| Loading files and libraries | `openat`, `read`, `fstat`, `mmap`, `mprotect` |
| Opening `test.txt` | `newfstatat`, `openat`, `faccessat` |
| Reading `test.txt` | `fstat`, `read`, `close` |
| Terminal configuration | `ioctl` |
| Waiting for user input | `ppoll`, `read` |
| Displaying the interface | `write` |
| Signal handling | `rt_sigaction`, `rt_sigprocmask` |
| Checking write permissions | `faccessat` |
| Saving `test.txt` | `openat`, `fstat`, `write`, `fsync`, `close` |
| Temporary file management | `newfstatat`, `openat`, `unlinkat` |
| Memory management | `mmap`, `munmap`, `mprotect`, `brk` |
| Program termination | `exit_group` |

---

## Conclusion

During this laboratory work, the system calls performed by GNU nano were analyzed using `strace`.

The experiment demonstrated that even a simple operation such as editing a text file involves many interactions with the Linux kernel.

The trace showed several categories of system calls, including file operations, memory management, terminal interaction, signal handling, permission checks, and process management.

The complete file-reading sequence was observed through calls such as:

```text
openat → fstat → read → close
```

The file-saving sequence was observed through:

```text
openat → fstat → write → fsync → close
```

The experiment also demonstrated the role of file descriptors. After a file is opened, operations such as `read()`, `write()`, `fstat()`, `fsync()`, and `close()` can operate on its file descriptor instead of its path.

As a result, `strace` provides a practical way to observe the boundary between a user-space application and the Linux kernel and to understand which operating-system services are required to implement application functionality.