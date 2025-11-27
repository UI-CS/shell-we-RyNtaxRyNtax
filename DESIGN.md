# 🏛️ Technical Design Document: Advanced vs. Standard Implementation

## 1. Executive Summary

This document outlines the architectural and algorithmic strategy adopted for the Unix Shell Project, contrasting it with the standard implementation implied by the project requirements. Our approach, rooted in professional software engineering principles, aims to deliver superior performance, maintainability, and concurrency by prioritizing **modular design**, **$O(1)$ algorithmic complexity**, and **lock-free synchronization** techniques.

## 2. Architectural Strategy: Modular Structure

The standard documentation implies a flatter structure where component files reside at the root or in simple subdirectories. We opt for a **Monorepo-style modular hierarchy** that enforces clear separation between reusable libraries, applications, and public interfaces. This decoupling ensures a scalable, maintainable, and library-ready codebase.

### 2.1 File System Hierarchy

| Feature            | Standard Implementation (Implied)                                  | Advanced Implementation (Selected)                                                                  | Rationale                                                                                              |
| :----------------- | :----------------------------------------------------------------- | :-------------------------------------------------------------------------------------------------- | :----------------------------------------------------------------------------------------------------- |
| **Project Layout** | Flat, execution-centric (files mixed at top-level/simple subdirs). | **Layered Hierarchy:** Separates source (`/src`), public headers (`/include`), and output (`/bin`). | Enforces **Separation of Concerns** (SoC) and promotes library reuse.                                  |
| **Headers**        | Localized or implicit headers.                                     | **Centralized API:** Public headers in `/include` define clear module interfaces.                   | Allows clear definition of module APIs, protecting the implementation details from external consumers. |
| **Shared Code**    | Scattered or duplicated across projects.                           | **Dedicated `/src/utils`:** Shared memory, atomic operations, and common functions are centralized. | Avoids code duplication and standardizes low-level utility implementations.                            |

---

## 3. Algorithmic Strategy: Core Shell Optimizations

The core shell is optimized to be **non-blocking** and **constant-time** in its essential operations, significantly improving responsiveness and minimizing overhead compared to standard linear or polling-based approaches.

### 3.1 Command Dispatch and History

| Component             | Standard Approach (Linear/Simple)                 | Advanced Approach (Optimized)             | Technical Advantage                                                                                                          |
| :-------------------- | :------------------------------------------------ | :---------------------------------------- | :--------------------------------------------------------------------------------------------------------------------------- |
| **Process Reaping**   | Polling `waitpid(..., WNOHANG)` in the main loop. | **Signal Handler (`SIGCHLD`)**            | Switches from wasteful CPU **polling** to efficient **event-driven** I/O. Guarantees immediate, non-blocking zombie cleanup. |
| **Built-in Dispatch** | Nested `if/else if` string comparisons.           | **Hash Table Lookup (Function Pointers)** | Reduces average lookup time from $O(N)$ (linear search) to **$O(1)$ (constant time)**.                                       |
| **Command History**   | Linear Array or Linked List.                      | **Circular Buffer (Array-based Queue)**   | Provides $O(1)$ time complexity for adding the latest command and efficient fixed-size memory management.                    |
| **Parsing**           | `strtok()` or simple loops.                       | **Finite State Automaton (FSA) Parser**   | Faster and more robust handling of complex syntax (e.g., quotes, escapes, I/O redirection tokens) in a single pass.          |

---

## 4. Algorithmic Strategy: Parallel Computing

For the optional projects, the strategy shifts focus to maximizing parallel speedup by minimizing synchronization overhead, primarily by replacing heavy operating system locks (Mutexes/Semaphores) with low-latency, hardware-optimized primitives.

### 4.1 Parallel Sudoku Validator

| Component       | Standard Approach (Implied)                                                | Advanced Approach (Optimized)        | Technical Advantage                                                                                                                    |
| :-------------- | :------------------------------------------------------------------------- | :----------------------------------- | :------------------------------------------------------------------------------------------------------------------------------------- |
| **Parallelism** | 11 threads (1 for rows, 1 for columns, 9 for subgrids).                    | **27 Threads (Maximum Parallelism)** | Assigns a unique thread to each of the 9 rows, 9 columns, and 9 subgrids, achieving maximum concurrent execution.                      |
| **Aggregation** | Shared global array updated with potential race conditions or Mutex locks. | **Atomic Flags / Bitmasks**          | Achieves **lock-free synchronization**. Threads use atomic operations to signal failure, eliminating kernel-level contention overhead. |

### 4.2 Monte Carlo Pi Estimation

| Component            | Standard Approach (Implied)                               | Advanced Approach (Optimized) | Technical Advantage                                                                                                                                                |
| :------------------- | :-------------------------------------------------------- | :---------------------------- | :----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Random Generator** | Standard Library `rand()` or `rand_r()`.                  | **Xorshift / PCG PRNG**       | Uses a faster, high-quality Pseudo-Random Number Generator (PRNG) optimized for parallel execution, ensuring better statistical distribution.                      |
| **Synchronization**  | Shared memory counter protected by Mutex/Semaphore locks. | **Atomic Accumulation**       | Replaces high-latency OS locks with lightweight **Atomic CPU instructions** (e.g., `__sync_fetch_and_add`), resulting in the fastest possible counter aggregation. |
