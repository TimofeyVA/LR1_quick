#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
using namespace std;

// Function to swap two numbers
void swap(int* arr, int i, int j)
{
    int t = arr[i];
    arr[i] = arr[j];
    arr[j] = t;
}

// Function that performs the Quick Sort
// for an array arr[] starting from the
// index start and ending at index end
void quicksort(int* arr, int start, int end)
{
    int pivot, index;

    // Base Case
    if (end <= 1)
        return;

    // Pick pivot and swap with first
    // element Pivot is middle element
    pivot = arr[start + end / 2];
    swap(arr, start, start + end / 2);

    // Partitioning Steps
    index = start;

    // Iterate over the range [start, end]
    for (int i = start + 1; i < start + end; i++) {

        // Swap if the element is less
        // than the pivot element
        if (arr[i] < pivot) {
            index++;
            swap(arr, i, index);
        }
    }

    // Swap the pivot into place
    swap(arr, start, index);

    // Recursive Call for sorting
    // of quick sort function
    quicksort(arr, start, index - start);
    quicksort(arr, index + 1, start + end - index - 1);
}

// Function that merges the two arrays
int* merge(int* arr1, int n1, int* arr2, int n2)
{
    int* result = (int*)malloc((n1 + n2) * sizeof(int));
    int i = 0;
    int j = 0;
    int k;

    for (k = 0; k < n1 + n2; k++) {
        if (i >= n1) {
            result[k] = arr2[j];
            j++;
        } else if (j >= n2) {
            result[k] = arr1[i];
            i++;
        }

        // Indices in bounds as i < n1
        // && j < n2
        else if (arr1[i] < arr2[j]) {
            result[k] = arr1[i];
            i++;
        }

        // v2[j] <= v1[i]
        else {
            result[k] = arr2[j];
            j++;
        }
    }
    return result;
}

// Driver Code
int main(int argc, char* argv[])
{
    srand(time(0));
    int number_of_elements;
    int* data = NULL;
    int chunk_size, own_chunk_size;
    int* chunk;
    double time_taken;
    MPI_Status status;

    int number_of_process, rank_of_process;
    int rc = MPI_Init(&argc, &argv);

    if (rc != MPI_SUCCESS) {
        printf("Error in creating MPI "
               "program.\n "
               "Terminating......\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &number_of_process);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank_of_process);

    if (rank_of_process == 0) {
        number_of_elements = atoi(argv[1]);

        cout << "Кол-во элементов: " << number_of_elements << endl;

        // Computing chunk size
        chunk_size
            = (number_of_elements % number_of_process == 0)
            ? (number_of_elements / number_of_process)
            : (number_of_elements / number_of_process
                - 1);

        data = (int*)malloc(number_of_process * chunk_size * sizeof(int));

        for (int i = 0; i < number_of_elements; i++) {
            data[i] = rand() % number_of_elements;
        }

        // Padding data with zero
        for (int i = number_of_elements; i < number_of_process * chunk_size; i++) {
            data[i] = 0;
        }

        cout << "Элементы массива: \n";
        for (int i = 0; i < number_of_elements; i++) {
            cout << data[i] << " ";
        }

        cout << endl;
    }

    // Blocks all process until reach this point
    MPI_Barrier(MPI_COMM_WORLD);

    // Starts Timer
    time_taken -= MPI_Wtime();

    // BroadCast the Size to all the
    // process from root process
    MPI_Bcast(&number_of_elements, 1, MPI_INT, 0,
        MPI_COMM_WORLD);

    // Computing chunk size
    chunk_size
        = (number_of_elements % number_of_process == 0)
        ? (number_of_elements / number_of_process)
        : number_of_elements
            / (number_of_process - 1);

    // Calculating total size of chunk
    // according to bits
    MPI_Barrier(MPI_COMM_WORLD);
    chunk = (int*)malloc(chunk_size * sizeof(int));

    // Scatter the chuck size data to all process
    MPI_Scatter(data, chunk_size, MPI_INT, chunk,
        chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    free(data);
    data = NULL;

    own_chunk_size = (number_of_elements
                         >= chunk_size * (rank_of_process + 1))
        ? chunk_size
        : (number_of_elements
            - chunk_size * rank_of_process);

    quicksort(chunk, 0, own_chunk_size);

    for (int step = 1; step < number_of_process;
         step = 2 * step) {
        if (rank_of_process % (2 * step) != 0) {
            MPI_Send(chunk, own_chunk_size, MPI_INT,
                rank_of_process - step, 0,
                MPI_COMM_WORLD);
            break;
        }

        if (rank_of_process + step < number_of_process) {
            int received_chunk_size
                = (number_of_elements
                      >= chunk_size
                          * (rank_of_process + 2 * step))
                ? (chunk_size * step)
                : (number_of_elements
                    - chunk_size
                        * (rank_of_process + step));
            int* chunk_received;
            chunk_received = (int*)malloc(
                received_chunk_size * sizeof(int));
            MPI_Recv(chunk_received, received_chunk_size,
                MPI_INT, rank_of_process + step, 0,
                MPI_COMM_WORLD, &status);

            data = merge(chunk, own_chunk_size,
                chunk_received,
                received_chunk_size);

            free(chunk);
            free(chunk_received);
            chunk = data;
            own_chunk_size
                = own_chunk_size + received_chunk_size;
        }
    }

    time_taken += MPI_Wtime();

    if (rank_of_process == 0) {

        cout << "Общее количество элементов, заданных в качестве входных данных : " << number_of_elements << endl;
        cout << " Отсортированный массив: " << endl;

        for (int i = 0; i < number_of_elements; i++) {
            cout << chunk[i] << " ";
        }

        cout << "\n\n Быстрая сортировка данных в" << number_of_process << " процедурах: " << time_taken << "секунд\n";
    }

    MPI_Finalize();
    return 0;
}