#include<stdio.h>
#include<omp.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>

#define NUMBER_OF_THREADS 8
#define ARRAY_MAX_SIZE 500000

int quick_arr[ARRAY_MAX_SIZE];
int quick_arr_sq[ARRAY_MAX_SIZE];
int merge_arr[ARRAY_MAX_SIZE];
int merge_arr_sq[ARRAY_MAX_SIZE];
int counting_arr[ARRAY_MAX_SIZE];
int counting_arr_sq[ARRAY_MAX_SIZE];
int counting_arr_sorted[ARRAY_MAX_SIZE];
int counting_arr_sq_sorted[ARRAY_MAX_SIZE];

void swap(int* a, int* b){
	int t = *a;
	*a = *b;
	*b = t;
}

void printArray(int arr[], int size){
	int i;
	for (i=0; i < size; i++)
		printf("%d ", arr[i]);
	printf("\n");
}

// QUICK SORT FUNCTIONS
int partition(int arr[], int low, int high){
	int pivot = arr[high];
	int i = (low - 1);
	for (int j = low; j <= high- 1; j++)
	{
		if (arr[j] <= pivot)
		{
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}

void quickSort(int arr[], int low, int high){
	if (low < high)
	{
		int pi = partition(arr, low, high);
		#pragma omp task firstprivate(arr,low,pi)
		{
			quickSort(arr,low, pi - 1);
		}
		#pragma omp task firstprivate(arr, high,pi)
		{
			quickSort(arr, pi + 1, high);
		}
	}
}

void run_quick_parallel(int quick_arr[], int n){
    #pragma omp parallel
    {
        int id = omp_get_thread_num();
        int nthrds = omp_get_num_threads();
        #pragma omp single nowait
        quickSort(quick_arr, 0, n-1);
    }
}

void run_quick_sequential(int quick_arr_sq[], int n){
    quickSort(quick_arr_sq, 0, n-1);
}

// MERGE SORT FUNCTIONS
void mergeSortAux(int *X, int n, int *tmp) {
   int i = 0;
   int j = n/2;
   int ti = 0;

   while (i<n/2 && j<n) {
      if (X[i] < X[j]) {
         tmp[ti] = X[i];
         ti++; i++;
      } else {
         tmp[ti] = X[j];
         ti++; j++;
      }
   }
   while (i<n/2) {
      tmp[ti] = X[i];
      ti++; i++;
   }
   while (j<n) {
      tmp[ti] = X[j];
      ti++; j++;
   }
   memcpy(X, tmp, n*sizeof(int));
} 

void mergeSort(int *X, int n, int *tmp){
   if (n < 2) return;

   #pragma omp task shared(X) if (n > 100)
   mergeSort(X, n/2, tmp);

   #pragma omp task shared(X) if (n > 100)
   mergeSort(X+(n/2), n-(n/2), tmp + n/2);

   #pragma omp taskwait
   mergeSortAux(X, n, tmp);
}

void run_merge_parallel(int merge_arr[], int n){
    int tmp[ARRAY_MAX_SIZE];
    #pragma omp parallel
    {
        #pragma omp single
        mergeSort(merge_arr, n, tmp);
    }
}

void run_merge_sequential(int merge_arr_sq[], int n){
    int tmp[ARRAY_MAX_SIZE];
    mergeSort(merge_arr, n, tmp);
}

// COUNTING SORT FUNCTIONS
void countingSort(int arr[], int arr_sorted[], int n) {
    int i, j, count;
    #pragma omp for private(i, j, count)
    for (i = 0; i < n; i++) {
        count = 0;
        for (j = 0; j < n; j++) {
            if (arr[i] > arr[j])
                count++;
        }
        while (arr_sorted[count] != 0)
            count++;
        arr_sorted[count] = arr[i];
    }
}

void run_counting_parallel(int counting_arr[], int counting_arr_sorted[], int n){
    #pragma omp parallel
    {
        countingSort(counting_arr, counting_arr_sorted, n);
    }   
}

void run_counting_sequential(int counting_arr[], int counting_arr_sorted[], int n){
    countingSort(counting_arr, counting_arr_sorted, n);   
}


int main()
{
	double start_time, run_time;
	for(int i = 0; i < ARRAY_MAX_SIZE; i++ ){
        int element = rand() % 50 + 1;
        quick_arr[i] = element;
        quick_arr_sq[i] = element;
        merge_arr[i] = element;
        merge_arr_sq[i] = element;
        counting_arr[i] = element;
        counting_arr_sq[i] = element;
        counting_arr_sorted[i] = 0;
        counting_arr_sq_sorted[i] = 0;
    }
	int n = sizeof(quick_arr)/sizeof(quick_arr[0]);

    omp_set_num_threads(NUMBER_OF_THREADS);

	// printf("Unsorted array: \n");
	// printArray(quick_arr, n);
	printf("Sorts with array of %d elements: \n", n);

    // ------------------------------   QUICK SORT < 500k   ------------------------------

    start_time = omp_get_wtime();
    run_quick_parallel(quick_arr, n);
	run_time = omp_get_wtime() - start_time;
	printf("Quick sort parallel execution time in seconds: %lf\n", run_time);
	// printf("Sorted array: \n");
	// printArray(quick_arr, n);

    start_time = omp_get_wtime();
	run_quick_sequential(quick_arr_sq, n);
	run_time = omp_get_wtime() - start_time;
	printf("Quick sort sequential execution time in seconds: %lf\n", run_time);
	// printf("Sorted array: \n");
	// printArray(quick_arr_sq, n);

    // ------------------------------   MERGE SORT   ------------------------------

    start_time = omp_get_wtime();
	run_merge_parallel(merge_arr, n);
	run_time = omp_get_wtime() - start_time;
	printf("Merge sort parallel execution time in seconds: %lf\n", run_time);
	// printf("Sorted array: \n");
	// printArray(merge_arr, n);

    start_time = omp_get_wtime();
	run_merge_sequential(merge_arr_sq, n);
	run_time = omp_get_wtime() - start_time;
	printf("Merge sort sequential execution time in seconds: %lf\n", run_time);
	// printf("Sorted array: \n");
	// printArray(merge_arr_sq, n);

    // ------------------------------   COUNTING SORT < 50k  ------------------------------
    if(ARRAY_MAX_SIZE <= 50000){
        start_time = omp_get_wtime();
        run_counting_parallel(counting_arr, counting_arr_sorted, n);
        run_time = omp_get_wtime() - start_time;
        printf("Counting sort parallel execution time in seconds: %lf\n", run_time);
        // printf("Sorted array: \n");
        // printArray(counting_arr_sorted, n);

        start_time = omp_get_wtime();
        run_counting_sequential(counting_arr_sq, counting_arr_sq_sorted, n);
        run_time = omp_get_wtime() - start_time;
        printf("Counting sort sequential execution time in seconds: %lf\n", run_time);
        // printf("Sorted array: \n");
        // printArray(counting_arr_sq_sorted, n);
    }

    // int x = 1;
    // for(int i = 0; i < n; i++){
    //     if(quick_arr[i] != counting_arr_sorted[i]){
    //         x = 0;
    //         break;
    //     }
    // }
    // if(x){
    //     printf("same\n");
    // }else{
    //     printf("not same\n");
    // }

	return 0;
}