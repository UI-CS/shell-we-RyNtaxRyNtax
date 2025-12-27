#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../include/utils/utils.h"

// Function prototypes
void merge(int arr[], int l, int m, int r);
void merge_sort(int arr[], int l, int r);

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

    printf("Original array: ");
    for (int i = 0; i < n; i++) 
    {
        printf("%d ", arr[i]);
    }
    printf("\n");

    // Perform the parallel merge sort
    merge_sort(arr, 0, n - 1);

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

void merge(int arr[], int l, int m, int r) 
{
    int i, j, k;
    int n1 = m - l + 1;
    int n2 = r - m;

    // Create temporary arrays to hold the two subarrays
    int L[n1], R[n2];

    for (i = 0; i < n1; i++) L[i] = arr[l + i];
    for (j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

    // Merge the temporary arrays back into arr[l..r]
    i = 0;
    j = 0;
    k = l;
    while (i < n1 && j < n2) 
    {
        if (L[i] <= R[j]) 
        {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy the remaining elements of L[], if there are any
    while (i < n1) 
    {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy the remaining elements of R[], if there are any
    while (j < n2) 
    {
        arr[k] = R[j];
        j++;
        k++;
    }
}

void merge_sort(int arr[], int l, int r) 
{
    if (l < r) 
    {
        int m = l + (r - l) / 2;

        pid_t pid1, pid2;

        // Create a child process to sort the first half
        pid1 = fork();
        if (pid1 < 0) 
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid1 == 0) 
        {
            // Child process sorts the first half and exits
            merge_sort(arr, l, m);
            exit(EXIT_SUCCESS);
        } else {
            // Create another child process to sort the second half
            pid2 = fork();
            if (pid2 < 0) 
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid2 == 0) 
            {
                // Child process sorts the second half and exits
                merge_sort(arr, m + 1, r);
                exit(EXIT_SUCCESS);
            } else {
                // Parent process waits for both children to complete
                waitpid(pid1, NULL, 0);
                waitpid(pid2, NULL, 0);
                // Merge the two sorted halves
                merge(arr, l, m, r);
            }
        }
    }
}

