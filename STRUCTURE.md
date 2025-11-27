# The Struture

```plaintext

/unix_shell_project
├── /src                          # All source code (C files)
│   ├── /shell                    # Core Unix Shell implementation files
│   │   ├── main.c                # Entry point, main loop, command dispatcher
│   │   ├── parser.c              # Advanced parsing logic (FSA/tokenization)
│   │   ├── builtins.c            # Built-in command functions (cd, exit, etc.)
│   │   ├── history.c             # History implementation (circular buffer)
│   │   └── proc_mgmt.c           # Process management (fork, exec, SIGCHLD)
│   │
│   │   └── /utils                # Reusable components used by multiple projects
│   │       ├── shared_mem.c      # mmap/shm_open shared memory helpers
│   │       └── atomic_ops.c      # High-performance atomic counters/flags
│   │
│   ├── /apps                     # Standalone executable applications (optional)
│   │   ├── sudoku.c              # Parallel Sudoku validator (pthreads)
│   │   └── pi_estimator.c        # Monte Carlo Pi estimator (process-based)
│   │
│   ├── /include                  # Header files
│   │   ├── /shell                # Shell headers
│   │   │   ├── shell.h
│   │   │   ├── parser.h
│   │   │   └── history.h
│   │   │
│   │   └── /utils
│   │       └── utils.h           # Common error codes, constants, helpers
│   │
│   └── /bin                      # Build output (executables)
│
├── Makefile                      # Build instructions
├── README.md                     # Project overview & usage
└── .gitignore                    # Exclude /bin, *.o, etc.
```
