#define _POSIX_C_SOURCE 200809L
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <math.h>
#include <stdatomic.h>
#include <pthread.h>
#include <sched.h>
#include <getopt.h>
#include "../../include/utils/atomic.h"
#include "../../include/utils/prng.h"

// Macros
#define DEFAULT_PROCESSES 4
#define DEFAULT_ITERATIONS 100000000L
#define DEFAULT_PRNG PCG32
#define CACHE_LINE_SIZE 64

// Statistics structure for each process
typedef struct 
{
    uint64_t inside_circle;
    uint64_t total_points;
    char _padding[CACHE_LINE_SIZE - (2 * sizeof(uint64_t))]; // Prevent false sharing
} process_stats_t;

// Command line options 
typedef struct
{
    int num_processes;
    int use_threads;
    long total_iterations;
    prng_type_t prng_type;
    int quiet_mode;
    int benchmark_mode;
} options_t;

// Function prototypes
static inline double get_time_sec(void);
static void parse_args(int argc, char *argv[], options_t *opts);
static void process_worker(int worker_id, uint64_t iterations, 
                            prng_type_t prng_type, 
                            process_stats_t *stats, 
                            uint64_t base_seed);
static void *thread_worker(void *arg);
static double monte_carlo_processes(options_t *opts);
static double monte_carlo_threads(options_t *opts);
static void calculate_error(double pi_estimate, uint64_t total_points,
                           double *error, double *confidence_interval);
static void print_results(double pi_estimate, double exec_time,
                         options_t *opts, uint64_t total_points);

int main(int argc, char *argv[]) 
{
    options_t opts;
    parse_args(argc, argv, &opts);
    
    if (!opts.quiet_mode && !opts.benchmark_mode) 
    {
        printf("Monte Carlo Pi Estimation\n");
        printf("=========================\n");
        printf("Configuring with %d %s...\n", 
                opts.num_processes,
                opts.use_threads ? "threads" : "processes");
    }
    
    // Start timing
    double start_time = get_time_sec();
    
    // Run Monte Carlo simulation
    double pi_estimate;
    if (opts.use_threads) 
    {
        pi_estimate = monte_carlo_threads(&opts);
    } else 
    {
        pi_estimate = monte_carlo_processes(&opts);
    }
    
    // End timing
    double end_time = get_time_sec();
    double exec_time = end_time - start_time;
    
    // Print results
    print_results(pi_estimate, exec_time, &opts, opts.total_iterations);
    
    return 0;
}


// Timing functions 
static inline double get_time_sec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Parse command line arguments
static void parse_args(int argc, char *argv[], options_t *opts)
{
    opts->num_processes = DEFAULT_PROCESSES;
    opts->use_threads = 0;
    opts->total_iterations = DEFAULT_ITERATIONS;
    opts->prng_type = DEFAULT_PRNG;
    opts->quiet_mode = 0;
    opts->benchmark_mode = 0;

    int opt;
    while ((opt = getopt(argc, argv, "p:t:i:r:qbh")) != -1)
    {
        switch (opt)
        {
        case 'p':
            opts->num_processes = atoi(optarg);
            if (opts->num_processes < 1)
            {
                fprintf(stderr, "Error: Number of processes must be <= 1\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'i':
            opts->total_iterations = atol(optarg);
            if (opts->total_iterations < 1)
            {
                fprintf(stderr, "Error: Iterations must be >= 1\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'r':
            if (strcmp(optarg, "pcg") == 0) opts->prng_type = PCG32;
            else if (strcmp(optarg, "xorshift") == 0) opts->prng_type = XORSHIFT128;
            else if (strcmp(optarg, "splitmix") == 0) opts->prng_type = SPLITMIX64;
            else
            {
                fprintf(stderr, "Error: Unknown PRNG type. Use pcg, xorshift or splitmix\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'q':
            opts->quiet_mode = 1;
            break;
        case 'b':
            opts->benchmark_mode = 1;
            break;
        case 'h':
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  -p N     Use N processes (default: %d)\n", DEFAULT_PROCESSES);
            printf("  -t N     Use N threads instead of processes\n");
            printf("  -i N     Total iterations (default: %ld)\n", DEFAULT_ITERATIONS);
            printf("  -r TYPE  PRNG type: pcg, xorshift, splitmix (default: pcg)\n");
            printf("  -q       Quiet mode (minimal output)\n");
            printf("  -b       Benchmark mode (CSV output)\n");
            printf("  -h       Show this help message\n");
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "Use -h for help\n");
            break;
        }
    }
}

// Worker function for process-based parallelism
static void process_worker(int worker_id, uint64_t iterations, prng_type_t prng_type,
                            process_stats_t *stats, uint64_t base_seed)
{
    prng_t prng;
    prng_init(&prng, base_seed + worker_id, prng_type);
    uint64_t inside = 0;

    for (uint64_t i = 0; i < iterations; i++)
    {
        // Generate random point in [0,1) x [0,1)
        double x = prng_next_double(&prng);
        double y = prng_next_double(&prng);

        // Check if points is inside unit circle 
        if (x * x + y * y <= 1.0) inside++;
    }

    stats[worker_id].inside_circle = inside;
    stats[worker_id].total_points = iterations;
}

// Thread worker function (for pthreads)
static void *thread_worker(void *arg)
{
    thread_worker_args_t *args = (thread_worker_args_t*)arg;
    prng_t prng;
    prng_init(&prng, args->seed, args->prng_type);
    uint64_t inside = 0;

    for (uint64_t i = 0; i < args->iterations; i++)
    {
        double x = prng_next_double(&prng);
        double y = prng_next_double(&prng);

        if (x * x + y * y <= 1.0) inside++;
    }

    atomic_fetch_add(args->total_inside, inside);
    atomic_fetch_add(args->total_points, args->iterations);

    return NULL;
}

// Process-based Monte Carlo implementation
static double monte_carlo_processes(options_t *opts)
{
    int num_workers = opts->num_processes;
    uint64_t total_iterations = opts->total_iterations;
    uint64_t base_iterations = total_iterations / num_workers;
    uint64_t remainder = total_iterations % num_workers;

    // Create shared memory for statistics
    size_t stats_size = num_workers * sizeof(process_stats_t);
    process_stats_t *shared_stats = mmap(NULL, stats_size,
                                        PROT_READ | PROT_WRITE,
                                        MAP_SHARED | MAP_ANONYMOUS,
                                        -1, 0);
    
    if (shared_stats == MAP_FAILED)
    {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    // Base seed from current time and PID
    uint64_t base_seed = (uint64_t)time(NULL) ^ (getpid() << 16);

    // Fork worker processes
    pid_t *pids = malloc(num_workers * sizeof(pid_t));

    for (int i = 0; i < num_workers; i++)
    {
        pids[i] = fork();

        if (pids[i] == 0)
        {
            // Child process
            uint64_t worker_iterations = base_iterations;
            if ( i == num_workers - 1)
            {
                worker_iterations += remainder; // Last worker gets remainder
            }

            process_worker(i, worker_iterations, opts->prng_type, shared_stats, base_seed);
            exit(EXIT_SUCCESS);
        }else if (pids[i] < 0)
        {
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }
    
    // Parent process: wait for all children
    for (int i = 0; i < num_workers; i++)
    {
        waitpid(pids[i], NULL, 0);
    }
    

    free(pids);

    // Aggrefate results
    uint64_t total_inside = 0;
    uint64_t total_points = 0;

    for (int i = 0; i < num_workers; i++)
    {
        total_inside += shared_stats[i].inside_circle;
        total_points += shared_stats[i].total_points;
    }

    // Cleanup
    munmap(shared_stats, stats_size);

    return 4.0 * (double)total_inside / (double)total_points;
}

// Thread-based Monte Carlo implementation
static double monte_carlo_threads(options_t *opts) 
{
    int num_threads = opts->num_processes; // Reusing field for thread count
    uint64_t total_iterations = opts->total_iterations;
    uint64_t base_iterations = total_iterations / num_threads;
    uint64_t remainder = total_iterations % num_threads;
    
    // Shared atomic counters
    _Atomic uint_fast64_t total_inside = ATOMIC_VAR_INIT(0);
    _Atomic uint_fast64_t total_points = ATOMIC_VAR_INIT(0);
    
    // Create thread arguments
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    thread_worker_args_t *args = malloc(num_threads * sizeof(thread_worker_args_t));
    
    // Base seed
    uint64_t base_seed = (uint64_t)time(NULL) ^ (getpid() << 16);
    
    for (int i = 0; i < num_threads; i++) 
    {
        args[i].seed = base_seed + i;
        args[i].prng_type = opts->prng_type;
        args[i].iterations = base_iterations;
        if (i == num_threads - 1) 
        {
            args[i].iterations += remainder;
        }
        args[i].total_inside = &total_inside;
        args[i].total_points = &total_points;
        
        // Set thread affinity for better cache performance (optional)
        pthread_create(&threads[i], NULL, thread_worker, &args[i]);
        
        // Set CPU affinity (Linux specific)
        #ifdef __linux__
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i % sysconf(_SC_NPROCESSORS_ONLN), &cpuset);
        pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset);
        #endif
    }
    
    // Wait for all threads
    for (int i = 0; i < num_threads; i++) 
    {
        pthread_join(threads[i], NULL);
    }
    
    // Calculate pi
    uint64_t inside = atomic_load(&total_inside);
    uint64_t points = atomic_load(&total_points);
    
    free(threads);
    free(args);
    
    return 4.0 * (double)inside / (double)points;
}

// Calculate statistical error
static void calculate_error(double pi_estimate, uint64_t total_points,
                            double *error, double *confidence_interval)
{
    double p = pi_estimate / 4.0; // Probability of point inside quarter circle
    double std_err = sqrt(p * (1.0 - p) / total_points);
    *error = 4.0 * std_err; // Error in pi estimate

    // 95% confidence interval (±2 standard errors)
    *confidence_interval = 1.96 * (*error); // 1.96 = 95% confidence for normal distribution

}

// Print results
static void print_results(double pi_estimate, double exec_time,
                        options_t *opts, uint64_t total_points)
{
    double error, confidence_interval;
    calculate_error(pi_estimate, total_points, &error, &confidence_interval);
    
    if (opts->benchmark_mode) {
        // CSV format for benchmarking
        printf("%s,%d,%ld,%.6f,%.10f,%.10f,%.3f\n",
                opts->use_threads ? "threads" : "processes",
                opts->num_processes,
                total_points,
                exec_time,
                pi_estimate,
                fabs(pi_estimate - M_PI),
                (double)total_points / exec_time / 1e6);
    } else if (!opts->quiet_mode) 
    {
        printf("\n=== Monte Carlo Pi Estimation ===\n");
        printf("Method: %s\n", opts->use_threads ? "Thread-based" : "Process-based");
        printf("Workers: %d\n", opts->num_processes);
        printf("PRNG: %s\n", 
                opts->prng_type == PCG32 ? "PCG32" :
                opts->prng_type == XORSHIFT128 ? "Xorshift128" : "SplitMix64");
        printf("Total points: %'lu\n", total_points);
        printf("\n");
        printf("Estimated π:  %.10f\n", pi_estimate);
        printf("Actual π:     %.10f\n", M_PI);
        printf("Error:        %.10f\n", fabs(pi_estimate - M_PI));
        printf("Std Error:    ±%.10f\n", error);
        printf("95%% CI:       ±%.10f\n", confidence_interval);
        printf("\n");
        printf("Execution time: %.3f seconds\n", exec_time);
        printf("Throughput: %.1f million points/sec\n", 
                (double)total_points / exec_time / 1e6);
        
        if (opts->num_processes > 1) 
        {
            double serial_time_est = exec_time * opts->num_processes * 0.8; // Estimate
            printf("Parallel efficiency: %.1f%%\n", 
                   (serial_time_est / exec_time) / opts->num_processes * 100);
        }
        printf("=================================\n");
    } else 
    {
        printf("%.10f (error: %.2e, time: %.3fs)\n", 
                pi_estimate, fabs(pi_estimate - M_PI), exec_time);
    }
}