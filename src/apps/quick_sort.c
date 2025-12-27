#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "../../include/utils/utils.h"

#define THRESHOLD 64

// Function prototypes
void swap(int *a, int *b);
int partition(int arr[], int low, int high);
void quick_sort(int arr[], int low, int high) ;


int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        fprintf(stderr, "Usage: %s <num1> <num2> ...\n", argv[0]);
        return 1;
    }

    int n = argc - 1;
    size_t size = n * sizeof(int);

    // Create a shared memory segment to store the array
    int *arr = create_shared_memory(size);
    if (arr == NULL) 
    {
        fprintf(stderr, "Failed to create shared memory.\n");
        return 1;
    }

    // Parse command-line arguments into the shared memory array
    for (int i = 0; i < n; i++) 
    {
        arr[i] = atoi(argv[i + 1]);
    }
    
    // Seed the random number generator once in the parent process
    srand(time(NULL));

    printf("Original array: ");
    for (int i = 0; i < n; i++) 
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
    fflush(stdout);

    // Perform the parallel quick sort
    quick_sort(arr, 0, n - 1);

    // Only the parent process should print the final sorted array
    if (getpid() > 0) 
    {
        printf("Sorted array: ");
        for (int i = 0; i < n; i++) 
        {
            printf("%d ", arr[i]);
        }
        printf("\n");
    }

    // Detach the shared memory segment
    if (detach_shared_memory(arr, size) == -1) 
    {
        fprintf(stderr, "Failed to detach shared memory.\n");
        return 1;
    }

    return 0;
}

void swap(int *a, int *b) 
{
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int arr[], int low, int high) 
{
    // Use a random pivot to avoid worst-case scenarios
    int random = low + rand() % (high - low);
    swap(&arr[random], &arr[high]);
    
    int pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) 
    {
        if (arr[j] < pivot) 
        {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quick_sort(int arr[], int low, int high) 
{
    if (low < high) 
    {
        int pi = partition(arr, low, high);

        // If the subarray is small, sort sequentially without forking
        if (high - low < THRESHOLD) 
        {
             // Sequential recursive calls
            quick_sort(arr, low, pi - 1);
            quick_sort(arr, pi + 1, high);
            return;
        }

        pid_t pid1, pid2;

        // Create a child process to sort the left subarray
        pid1 = fork();
        if (pid1 < 0) 
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid1 == 0) 
        {
            // Child process sorts the left subarray and exits
            quick_sort(arr, low, pi - 1);
            exit(EXIT_SUCCESS);
        }

        // Create another child process to sort the right subarray
        pid2 = fork();
        if (pid2 < 0) 
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid2 == 0) 
        {
            // Child process sorts the right subarray and exits
            quick_sort(arr, pi + 1, high);
            exit(EXIT_SUCCESS);
        }
        
        // Parent process waits for both children to complete
        waitpid(pid1, NULL, 0);
        waitpid(pid2, NULL, 0);
    }
}
