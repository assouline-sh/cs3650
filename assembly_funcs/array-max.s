# Write the assembly code for array_max
# Usage: ./array-max <arg1> <arg2> ... <argn>

# Make `main` accessible outside of this module
.global array_max

# Start of the code section
.text

# SIGNATURE: int array_max(unsigned long n, unsigned long * x)
# PSEUDOCODE:
# int array_max(unsigned long n, unsigned long * x) {
#  max = x;
#  counter = 1;
#  while (counter < n) {
#	if (args[couter] > max) {
#	   max = args[counter];
#       }
#       counter = counter + 1;
#  }
#  printf(max);
#  return 0;
# }
# VARIABLE MAPPINGS:
# n -> %r12
# x -> %r13
# counter -> %r14
# max -> %r15
# SKELETON:
# main:
# Prologue: set up stack frame
# 	enter $0, $0
# Body: TODO:
# 	define variables
#	loop through arguments, assigning the greatest one visited yet as maximum
#	when all arguments are visited, return the maximum 
# Epilogue:
#	clean up stack frame
#	leave
#	ret
# WRITE THE BODY:

# int array_max(unsigned long, unsigned long *)
array_max:
   # Function prologue 
   mov %rdi, %r12	# number of arguments
   mov %rsi, %r13	# address of first element
   mov $1, %r14		# counter
   mov (%r13), %r15	# initialize the register to store the current maximum as the first element

   # Loop through the arguments to find a new maximum, if there is a valid one
   loop:
	cmp %r12, %r14
	jge loop_end

	cmp (%r13, %r14, 8), %r15
	#cmovg (%r13, %r14, 8), %r15
	jl set_max
	
	inc %r14
	jmp loop

   set_max:
	mov $0, %r15
	mov (%r13, %r14, 8), %r15
	inc %r14
	jmp loop

   loop_end:
	mov $format, %rdi
        mov %r15, %rsi
        mov $0, %al
        call printf
	leave
	ret
   
   # Function epilogue
   leave
   ret

.data
format:
   .asciz "%ld\n"	
