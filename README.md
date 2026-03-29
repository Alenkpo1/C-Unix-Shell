# C-Unix-Shell

A basic command-line interpreter (Shell) for Unix systems, developed entirely in C. This project demonstrates low-level process management, inter-process communication (IPC), and core operating system concepts.

## Key Features

* **Command Execution**: Ability to execute external binary programs located in any system path defined by the `$PATH` environment variable.
* **Background Processes**: Support for running commands in the background by appending `&` at the end of the command.
* **Multiple Pipes**: Connecting the standard output of one process to the standard input of the next using `|` (e.g., `ls -l | grep txt`).
* **Stream Redirection**: 
  * Standard output redirection to files (`>`).
  * Standard error redirection and its combination with standard output (`2>&1`).
* **Temporary Environment Variables**: Support for defining temporary variables when executing a command (e.g., `VAR=value command`).
* **Pseudo-variables**: Expansion of special environment variables (like `$?`) to check the exit status of executed programs.
* **Built-in Commands**: Native implementation of essential commands that cannot be delegated to external executables, such as `cd` (change directory) and `pwd` (print working directory).

## Requirements

- Unix/Linux or MacOS environment (for POSIX system calls and the `execve`/`fork` family).
- C Compiler (GCC or Clang).
- `make` utility.

## Build and Run

To build the shell executable, clone the repository and run the `make` command:

```bash
make
```

To enter the interactive shell environment:

```bash
./sh
```

## Testing

The repository comes with an integrated test suite that verifies the correct functionality of redirections, pipes, and processes.

- To run all tests:
```bash
make test
```

- To run a single test (where `TEST_NAME` is the name of the test):
```bash
make test-TEST_NAME
```
*(e.g.: `make test-env_empty_variable`)*

## Documentation and Theory

The file [`shell.md`](shell.md) contains detailed answers in Spanish to the theoretical concepts investigated for this project, such as the difference between `execve(2)` and C wrappers, orphan process management, and file descriptor level redirections.

---
*This project was originally developed as part of an Operating Systems university assignment | FIUBA.*
