#
# Usage: ./calculator <op> <arg1> <arg2>
#

# Make `main` accessible outside of this module
.global main

# Start of the code section
.text

# int main(int argc, char argv[][])
main:
  # Function prologue
  enter $0, $0

  # Variable mappings:
  # op -> %r12
  # arg1 -> %r13
  # arg2 -> %r14
  movq 8(%rsi), %r12  # op = argv[1]
  movq 16(%rsi), %r13 # arg1 = argv[2]
  movq 24(%rsi), %r14 # arg2 = argv[3]

  
  # Hint: Convert 1st operand to long int
  movq %r13, %rdi
  call atol
  movq %rax, %r13
  
  # Hint: Convert 2nd operand to long int
  movq %r14, %rdi
  call atol
  movq %rax, %r14

  # Hint: Copy the first char of op into an 8-bit register
  # i.e., op_char = op[0] - something like mov 0(%r12), ???
  mov 0(%r12), %al 

  # Compare operator to +,-,*,/ and call the corresponding function to 
  # perform the operation; if it does not match a valid symbol print an error
  cmp $'+', %al
  je add
  cmp $'-', %al
  je sub
  cmp $'*', %al
  je mul
  cmp $'/', %al
  je div

  # when no valid operator given, print an error message of Unknown operator
  jmp error

  # Add the two arguments
  add:
	movq %r13, %rbx
	add %r14, %rbx
	jmp print

  # Subtract the two arguments
  sub:
	movq %r13, %rbx
	sub %r14, %rbx
	jmp print

  # Multiply the two arguments
  mul:
	movq %r13, %rbx
	imul %r14, %rbx
	jmp print

  # Divide the two arguments. If dividing by 0, throw an error	
  div:
	mov %r13, %rax
	xor %rdx, %rdx
	cqo
	cmp $0, %r14
	je error0
	idiv %r14
	mov %rax, %rbx
	jmp print 

  # Print the answer in long format
  print: 
	mov $format, %rdi
	mov %rbx, %rsi
	mov $0, %al
	call printf	
	leave
	ret
 
  # Print an error message for unknown operation
  error:
        mov $error_message, %rdi
        mov $0, %al
        call printf

  # Print an error message for dividing by 0
  error0:
	mov $error_message0, %rdi
	mov $0, %al
	call printf

  # Function epilogue
  leave
  ret


# Start of the data section
.data

format: 
  .asciz "%ld\n"
error_message:
  .asciz "Unknown operation\n"
error_message0:
  .asciz "Cannot divide by zero\n"

