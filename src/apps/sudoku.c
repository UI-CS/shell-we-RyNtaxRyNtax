#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "../../include/utils/atomic.h"
#include "../../include/utils/utils.h"

#define GRID_SIZE 9
#define SUBGRID_SIZE 3
#define NUM_THREADS 27 // 9 rows + 9 columns + 9 subgrids

// Global validation results (atomic flags for lock-free synchronization)
static atomic_flag row_flags[GRID_SIZE];
static atomic_flag col_flags[GRID_SIZE];
static atomic_flag grid_flags[GRID_SIZE];
static atomic_int valid_count = ATOMIC_VAR_INIT(0);

// Thread data structure
typedef struct
{
    int id;
    int (*puzzle)[GRID_SIZE];
    int row;
    int col;
    int type; // 0=row, 1=column, 2=subgrid
} sudoku_thread_t;

// Function prototypes
static bool check_bitset(const int region[GRID_SIZE]);
void *validate_region(void *arg);
void init_atomic_flags(void);
bool parallel_sudoku_validate(int puzzle[GRID_SIZE][GRID_SIZE]);
void print_sudoku(int puzzle[GRID_SIZE][GRID_SIZE], const char* title);


int main(void) 
{
    printf("=== Parallel Sudoku Validator ===\n");
    printf("Threads: %d (9 rows + 9 columns + 9 subgrids)\n", NUM_THREADS);
    printf("Synchronization: Atomic Flags (lock-free)\n\n");
    
    // Valid Sudoku puzzle
    int valid_puzzle[GRID_SIZE][GRID_SIZE] = {
        {5, 3, 4, 6, 7, 8, 9, 1, 2},
        {6, 7, 2, 1, 9, 5, 3, 4, 8},
        {1, 9, 8, 3, 4, 2, 5, 6, 7},
        {8, 5, 9, 7, 6, 1, 4, 2, 3},
        {4, 2, 6, 8, 5, 3, 7, 9, 1},
        {7, 1, 3, 9, 2, 4, 8, 5, 6},
        {9, 6, 1, 5, 3, 7, 2, 8, 4},
        {2, 8, 7, 4, 1, 9, 6, 3, 5},
        {3, 4, 5, 2, 8, 6, 1, 7, 9}
    };
    
    // Invalid puzzle (duplicate in first row)
    int invalid_puzzle[GRID_SIZE][GRID_SIZE] = {
        {5, 5, 4, 6, 7, 8, 9, 1, 2},  // Duplicate 5 in row 0
        {6, 7, 2, 1, 9, 5, 3, 4, 8},
        {1, 9, 8, 3, 4, 2, 5, 6, 7},
        {8, 5, 9, 7, 6, 1, 4, 2, 3},
        {4, 2, 6, 8, 5, 3, 7, 9, 1},
        {7, 1, 3, 9, 2, 4, 8, 5, 6},
        {9, 6, 1, 5, 3, 7, 2, 8, 4},
        {2, 8, 7, 4, 1, 9, 6, 3, 5},
        {3, 4, 5, 2, 8, 6, 1, 7, 9}
    };
    
    // Test valid puzzle
    print_sudoku(valid_puzzle, "Test 1: Valid Sudoku");
    
    clock_t start = clock();
    bool result1 = parallel_sudoku_validate(valid_puzzle);
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Parallel Validation Result: %s\n", result1 ? "VALID ✓" : "INVALID ✗");
    printf("Time taken: %.6f seconds\n\n", time_taken);
    
    // Test invalid puzzle
    print_sudoku(invalid_puzzle, "Test 2: Invalid Sudoku (duplicate in row)");
    
    start = clock();
    bool result2 = parallel_sudoku_validate(invalid_puzzle);
    end = clock();
    time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Parallel Validation Result: %s\n", result2 ? "VALID ✓" : "INVALID ✗");
    printf("Time taken: %.6f seconds\n\n", time_taken);
    
    // Edge case: empty grid
    int empty_grid[GRID_SIZE][GRID_SIZE] = {0};
    printf("\nEdge Case: Empty Grid (All zeros)\n");
    printf("---------------------------------\n");
    printf("Parallel Validation Result: %s\n", 
        parallel_sudoku_validate(empty_grid) ? "VALID ✓" : "INVALID ✗");
    
    printf("\n=== Parallel Sudoku Validator Complete ===\n");
    return 0;
}

// Check if array contains 1-9 exactly once (using bitmask for 0-1 operations)
static bool check_bitset(const int region[GRID_SIZE])
{
    int bitset = 0;

    for (int i = 0; i < GRID_SIZE; i++)
    {
        int num = region[i];
        if (num < 1 || num > GRID_SIZE) return false;

        int bit = 1 << (num - 1);
        if (bitset & bit) return false; // Duplicate found
        
        bitset |= bit;
    }

    // Check if all bits 0-8 are set (all numbers 1-9 present)
    return bitset == 0x1FF; // 0x1FF = 111111111 in binary (9 bits)
}

// Thread function for parallel validation
void *validate_region(void *arg)
{
    sudoku_thread_t *data = (sudoku_thread_t*)arg;
    int region[GRID_SIZE];
    bool valid = true;

    if (data->type == 0) // Validate row (type == 0)
    {
        for (int col = 0; col < GRID_SIZE; col++)
        {
            region[col] = data->puzzle[data->row][col];
        }

        valid = check_bitset(region);
        if (valid)
        {
            atomic_flag_test_and_set(&row_flags[data->row]);
            atomic_fetch_add(&valid_count, 1);
        }
    } 
    else if (data->type == 1) // Validate column (type == 1)
    {
        for (int row = 0; row < GRID_SIZE; row++)
        {
            region[row] = data->puzzle[row][data->col];
        }

        valid = check_bitset(region);
        if (valid)
        {
            atomic_flag_test_and_set(&col_flags[data->col]);
            atomic_fetch_add(&valid_count, 1);
        }
    }
    else // Validate subgrid (type == 2)
    {
        int index = 0;
        for (int i = 0; i < SUBGRID_SIZE; i++)
        {
            for (int j = 0; j < SUBGRID_SIZE; j++)
            {
                region[index++] = data->puzzle[data->row + i][data->col + j];
            }
        }

        valid = check_bitset(region);
        int grid_index = (data->row / SUBGRID_SIZE) * 3 + (data->col / SUBGRID_SIZE);
        if (valid)
        {
            atomic_flag_test_and_set(&grid_flags[grid_index]);
            atomic_fetch_add(&valid_count, 1);
        }
    }

    free(data);
    return NULL;
}

// Initialize atomic flags
void init_atomic_flags(void)
{
    for (int i = 0; i < GRID_SIZE; i++)
    {
        atomic_flag_clear(&row_flags[i]);  
        atomic_flag_clear(&col_flags[i]);  
        atomic_flag_clear(&grid_flags[i]);
    }
    atomic_store(&valid_count, 0);
}

// Parallel Sudoku validation using 27 threads
bool parallel_sudoku_validate(int puzzle[GRID_SIZE][GRID_SIZE])
{
    pthread_t threads[NUM_THREADS];
    int thread_index = 0;

    // Initialize atomic flags
    init_atomic_flags();

    // Create threads for rows
    for (int row = 0; row < GRID_SIZE; row++)
    {
        sudoku_thread_t *data = malloc(sizeof(sudoku_thread_t));
        data->id = thread_index++;
        data->puzzle = puzzle;
        data->row = row;
        data->col = 0;
        data->type = 0;

        pthread_create(&threads[data->id], NULL, validate_region, data);
    }

    // Create threads for columns
    for (int col = 0; col < GRID_SIZE; col++) 
    {
        sudoku_thread_t* data = malloc(sizeof(sudoku_thread_t));
        data->id = thread_index++;
        data->puzzle = puzzle;
        data->row = 0;
        data->col = col;
        data->type = 1;

        pthread_create(&threads[data->id], NULL, validate_region, data);
    }

    // Create threads for subgrids
    for (int block_row = 0; block_row < GRID_SIZE; block_row += SUBGRID_SIZE) 
    {
        for (int block_col = 0; block_col < GRID_SIZE; block_col += SUBGRID_SIZE) 
        {
            sudoku_thread_t* data = malloc(sizeof(sudoku_thread_t));
            data->id = thread_index++;
            data->puzzle = puzzle;
            data->row = block_row;
            data->col = block_col;
            data->type = 2;
            
            pthread_create(&threads[data->id], NULL, validate_region, data);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    // Check results - ALL 27 threads must have reported valid
    bool all_valid = (atomic_load(&valid_count) == NUM_THREADS);

    // Also verify that all atomic flags are set (double-check)
    for (int i = 0; i < GRID_SIZE; i++)
    {
        if (!atomic_flag_test_and_set(&row_flags[i]) ||  
            !atomic_flag_test_and_set(&col_flags[i]) ||   
            !atomic_flag_test_and_set(&grid_flags[i]))
        {
            all_valid = false;
        }
    }

    return all_valid;
}

// Print Sudoku grid
void print_sudoku(int puzzle[GRID_SIZE][GRID_SIZE], const char* title) 
{
    printf("\n%s\n", title);
    printf("+-------+-------+-------+\n");
    
    for (int i = 0; i < GRID_SIZE; i++) 
    {
        printf("| ");
        for (int j = 0; j < GRID_SIZE; j++) 
        {
            printf("%d ", puzzle[i][j]);
            if ((j + 1) % 3 == 0) printf("| ");
        }
        printf("\n");
        
        if ((i + 1) % 3 == 0 && i != GRID_SIZE - 1) 
        {
            printf("+-------+-------+-------+\n");
        }
    }
    printf("+-------+-------+-------+\n");
}
