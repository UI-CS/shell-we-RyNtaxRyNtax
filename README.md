# Unix Shell Project

A feature-rich Unix shell implementation in C, featuring advanced process management, parallel computing applications, and optimized algorithms.

## Features

### Core Shell

- **Command Execution**: Supports standard Unix commands.
- **Pipeline Support**: Execute multiple commands piped together (e.g., `ls | grep txt`).
- **I/O Redirection**: Support for input (`<`) and output (`>`) redirection.
- **Background Execution**: Run commands in background using `&`.
- **History**: Access command history with `history` and execute last command with `!!`.
- **Built-ins**: `cd`, `pwd`, `exit`, `export`, `unset`, `help`.
- **Process Management**: Event-driven zombie reaping using `SIGCHLD` signals.

### Parallel Applications

Included in `bin/`:

- **Sudoku Validator**: Validates 9x9 Sudoku puzzles using 27 concurrent threads (lock-free atomic synchronization).
- **Pi Estimator**: Estimates $\pi$ using Monte Carlo method (Process and Thread modes supported).
- **Parallel Merge Sort**: Fork-Join merge sort with shared memory and recursion threshold.
- **Parallel Quick Sort**: Fork-Join quick sort with shared memory and recursion threshold.

## Compilation

The project uses a `Makefile` for easy compilation.

```bash
# Compile everything (Shell + Apps)
make

# Compile only the shell
make bin/unixsh

# Clean build artifacts
make clean
```

**Requirements:**

- GCC Compiler
- GNU Make
- `libreadline-dev` (for command history editing)

## Usage

### Running the Shell

```bash
./bin/unixsh
```

Once inside the shell:

```bash
unixsh> ls -l | grep .c
unixsh> sleep 5 &
unixsh> ./bin/pi_estimator -p 4 -i 1000000
```

### Running Applications Independently

**Sudoku Validator:**

```bash
./bin/sudoku_validator
```

**Pi Estimator:**

```bash
# Run with 4 processes, 100 million iterations
./bin/pi_estimator -p 4 -i 100000000

# Run with 8 threads
./bin/pi_estimator -t 8 -i 100000000
```

**Merge Sort / Quick Sort:**

```bash
./bin/merge_sort 10 5 2 9 1 8
./bin/quick_sort 10 5 2 9 1 8
```

## Testing

Run the automated test suite:

```bash
make test
```

This will run basic checks for the shell and applications.
