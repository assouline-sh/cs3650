/* Complete the C version of the driver program for compare. This C code does
 * not need to compile. */

#include <stdio.h>

extern long compare(long, long);

int main(int argc, char *argv[]) {
  /* If less than three arguments are given, throw an error */
  if (argc < 3) {
	fprintf(stderr, "Not enough arguments")
	return 1;
  }
  
  /* Convert both arguments to long ints */
  unsigned long nums[argc - 1];
  for (int i = 1; i < arg; ++i) {
	nums[i - 1] = atol(argv[i])
  }
  
  /* Call compare function with both arguments and store the result */
  int result;
  result = compare(nums[0], nums[1])
  
  if (result == -1) {
	printf("less");
	return 0;
  }
  else if (result == 0) {
	printf("equal");
	return 0;
  }
  else if (result == 1) {
	printf("greater");
	return 0;
  }
	
}

