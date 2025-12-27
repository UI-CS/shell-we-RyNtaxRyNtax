# The Struture

```plaintext
/unix_shell_project
├── /src                          # All source code (C files)
│   ├── /shell                    # Core Unix Shell implementation files
│   │   ├── main.c                # Entry point, main loop, command dispatcher
│   │   ├── parser.c              # Advanced parsing logic (FSA/tokenization)
│   │   ├── executor.c            # Pipeline execution, redirection, and fork/exec logic
│   │   ├── builtins.c            # Built-in command functions (cd, exit, history, etc.)
│   │   ├── history.c             # History implementation (circular buffer)
│   │   └── proc_mgmt.c           # Process management (SIGCHLD handler and reaping)
│   │
│   └── /utils                    # Reusable components used by multiple projects
│       ├── prng.c                # Pseudo-Random Number Generator implementations
│       ├── shared_mem.c          # mmap/shm_open shared memory helpers
│       └── atomic_ops.c          # High-performance atomic counters/flags
│
├── /apps                         # Standalone executable applications
│   ├── sudoku.c                  # Parallel Sudoku validator (pthreads)
│   └── pi_estimator.c            # Monte Carlo Pi estimator (process-based)
│
├── /include                      # Header files
│   ├── /shell                    # Shell headers
│   │   ├── shell.h               # Global shell definitions
│   │   ├── parser.h              # Command structures and parsing prototypes
│   │   ├── builtins.h            # Built-in function mappings
│   │   └── history.h             # History buffer definitions
│   │
│   └── /utils                    # Utility headers
│       ├── atomic.h              # Atomic wrapper functions
│       ├── prng.h                # Random generator states and interfaces
│       └── utils.h               # Common macros and shared memory prototypes
│
├── /bin                          # Build output (executables - gitignored)
├── Makefile                      # Build instructions
├── DESIGN.md                     # Project design philosophy and phases
├── README.md                     # Project overview and usage
└── STRUCTURE.md                  # This file
```
