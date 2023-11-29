# Write the assembly code for main
# Usage: ./compare <num1> <num2>

# Make `main` accessible outside of this module
.global main

# Start of the code section
.text

# SIGNATURE: int main(int argc, char argv[])
# PSEUDOCODE:
# int main(int argc, char *argv[]) {
#  if (argc < 3) {
#	printf(stderr, "Not enough arguments")
#	return 1;
#  }
#  unsigned long nums[argc - 1];
#  for (int i = 1; i < arg; ++i) {
#	nums[i - 1] = atol(argv[i])
#  }
#  int result;
#  result = compare(nums[0], nums[1])
#  if (result == -1) {
#	printf("less");
#	return 0;
#  }
#  else if (result == 0) {
#	printf("equal");
#	return 0;
#  }
#  else if (result == 1) {
#	printf("greater");
#	return 0;
#  }
# }
# VARIABLE MAPPINGS:
# argc -> %r12
# nums[0] -> %r13
# nums[1] -> %r14
# result -> %rbx
# SKELETON:
# main:
# Prologue: set up stack frame
# 	enter $0, $0
# Body: TODO:
# 	define variables
#	check total argument number is 3, including program name
#	convert both arguments to long ints
#	call compare function
#	print out less, equal, or greater accordingly to result from compare
# Epilogue:
#	clean up stack frame
#	leave
#	ret
# WRITE THE BODY:

main:
   # PROLOGUE
   enter $0, $0

   # BODY
   # Variable Mappings
   mov %rdi, %r12	# number of args
   movq 8(%rsi), %r13	# num1 = argv[1]
   movq 16(%rsi), %r14  # num2 = argv[2]

   # Check that number of arguments is 2, if not return an error
   cmp $3, %r12
   jne error
 
   # Convert arguments to long int
   movq %r13, %rdi
   call atol
   movq %rax, %r13
 
   movq %r14, %rdi
   call atol
   movq %rax, %r14

   # Call the provided compare function with %r12 and %r13
   movq %r13, %rdi
   movq %r14, %rsi
   call compare
   movq %rax, %rbx
   
   # Call the appropriate function based on the result of compare
   cmp $-1, %rbx
   je less
   cmp $0, %rbx
   je equal
   cmp $1, %rbx
   je greater

   # Functions for less than, equal to, greater than, printing, and throwing an error
   less:
	movq $less_than, %rdi
        jmp print

   equal:
	movq $equal_to, %rdi
	jmp print

   greater:
	movq $greater_than, %rdi
	jmp print
   
   print:
	mov $0, %al
	call printf
	leave
	ret
   
   error:
	mov $error_message, %rdi
 	mov $1, %al
	call printf
	leave 
	ret

   # EPILOGUE
   leave
   ret

.data
less_than:
   .asciz "less\n"
equal_to:
   .asciz "equal\n"
greater_than:
   .asciz "greater\n"
error_message:
   .asciz "Two arguments required.\n"
