#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#define tty_printf(...) (isatty(1) && isatty(0) ? printf(__VA_ARGS__) : 0)

#ifndef SHUSH
#define log(...) (fprintf(stderr, __VA_ARGS__))
#else 
#define log(...)
#endif


/** The number of threads to be used for sorting. Default: 1 */
int max_thread_count = 1;

/** Keep track of number of threads running to ensure it does not surpass maximum number of threads to run */
int current_thread_count = 1;

// Helper struct to pass custom arguments to pthread_create
typedef struct args {
  long *nums;
  int from;
  int to; 
  long *target;
} args;


/**
 * Compute the delta between the given timevals in seconds.
 */
double time_in_secs(const struct timeval *begin, const struct timeval *end) {
  long s = end->tv_sec - begin->tv_sec;
  long ms = end->tv_usec - begin->tv_usec;
  return s + ms * 1e-6;
}


/**
 * Print the given array of longs, an element per line.
 */
void print_long_array(const long *array, int count) {
  for (int i = 0; i < count; ++i) {
    printf("%ld\n", array[i]);
  }
}


/**
 * Merge two slices of nums into the corresponding portion of target.
 */
void merge(long nums[], int from, int mid, int to, long target[]) {
  int left = from;
  int right = mid;

  int i = from;
  for (; i < to && left < mid && right < to; i++) {
    if (nums[left] <= nums[right]) {
      target[i] = nums[left];
      left++;
    }
    else {
      target[i] = nums[right];
      right++;
    }
  }
  if (left < mid) {
    memmove(&target[i], &nums[left], (mid - left) * sizeof(long));
  }
  else if (right < to) {
    memmove(&target[i], &nums[right], (to - right) * sizeof(long));
  }
}


/**
 * Sort the given slice of nums into target.
 *
 * Warning: nums gets overwritten.
 */
void* merge_sort_aux(void *arg_in) {
  struct args *arg = arg_in;
  long *nums = arg->nums; 
  int from = arg->from;
  int to = arg->to;
  long *target = arg->target;

  free(arg_in);

  if (to - from <= 1) {
    return NULL;
  }

  // point to middle of array
  int mid = (from + to) / 2;

  // if we have the capacity to create 2 more threads, run merge_sort_aux concurrently 
  // else run normally
  if (current_thread_count + 2 < max_thread_count) {
    current_thread_count += 2;
    
    // set arguments for the left side of the array
    args *arguments = (args *)malloc(sizeof(struct args));
    arguments->nums = target;
    arguments->from = from;
    arguments->to = mid;
    arguments->target = nums;

    //create a new thread to run merge_sort_aux on left side of array
    pthread_t tid;
    pthread_create(&tid, NULL, merge_sort_aux, arguments);

    // set arguments for the right side of the array
    args *arguments2 = (args *)malloc(sizeof(struct args));
    arguments2->nums = target;
    arguments2->from = mid;
    arguments2->to = to;
    arguments2->target = nums;

    //create a new thread to run merge_sort_aux on right side of array
    pthread_t tid2;
    pthread_create(&tid2, NULL, merge_sort_aux, arguments2);

    // join the threads
    pthread_join(tid, NULL);
    pthread_join(tid2, NULL);

    // threads are finished, decrement current thread count
    current_thread_count -= 2;

    // merge the two lists
    merge(nums, from, mid, to, target);
  }
  else {
    // initialize arguments for left side
    args *arguments = (args *)malloc(sizeof(struct args));
    arguments->nums = target;
    arguments->from = from;
    arguments->to = mid;
    arguments->target = nums;

    // initialize arguments for right side
    args *arguments2 = (args *)malloc(sizeof(struct args));
    arguments2->nums = target;
    arguments2->from = mid;
    arguments2->to = to;
    arguments2->target = nums;
    
    // run merge sort normally
    merge_sort_aux(arguments);
    merge_sort_aux(arguments2);
    merge(nums, from, mid, to, target);
  }
  return NULL;
}


/**
 * Sort the given array and return the sorted version.
 *
 * The result is malloc'd so it is the caller's responsibility to free it.
 *
 * Warning: The source array gets overwritten.
 */
long *merge_sort(long nums[], int count) {
  long *result = calloc(count, sizeof(long));
  assert(result != NULL);
  memmove(result, nums, count * sizeof(long));

  int current_thread_count = 1;
  args *arguments = (args *)malloc(sizeof(struct args));
  arguments->nums = nums;
  arguments->from = 0;
  arguments->to = count;
  arguments->target = result;

  merge_sort_aux(arguments);

  return result;
}


/**
 * Based on command line arguments, allocate and populate an input and a 
 * helper array.
 *
 * Returns the number of elements in the array.
 */
int allocate_load_array(int argc, char **argv, long **array) {
  assert(argc > 1);
  int count = atoi(argv[1]);

  *array = calloc(count, sizeof(long));
  assert(*array != NULL);

  long element;
  tty_printf("Enter %d elements, separated by whitespace\n", count);
  int i = 0;
  while (i < count && scanf("%ld", &element) != EOF)  {
    (*array)[i++] = element;
  }

  return count;
}


int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <n>\n", argv[0]);
    return 1;
  }

  struct timeval begin, end;

  // get the number of threads from the environment variable SORT_THREADS
  if (getenv("MSORT_THREADS") != NULL)
    max_thread_count = atoi(getenv("MSORT_THREADS"));

  log("Running with %d thread(s). Reading input.\n", max_thread_count);

  // Read the input
  gettimeofday(&begin, 0);
  long *array = NULL;
  int count = allocate_load_array(argc, argv, &array);
  gettimeofday(&end, 0);

  log("Array read in %f seconds, beginning sort.\n", 
      time_in_secs(&begin, &end));
 
  // Sort the array
  gettimeofday(&begin, 0);
  long *result = merge_sort(array, count);
  gettimeofday(&end, 0);
  
  log("Sorting completed in %f seconds.\n", time_in_secs(&begin, &end));

  // Print the result
  gettimeofday(&begin, 0);
  print_long_array(result, count);
  gettimeofday(&end, 0);
  
  log("Array printed in %f seconds.\n", time_in_secs(&begin, &end));

  free(array);
  free(result);

  return 0;
}
