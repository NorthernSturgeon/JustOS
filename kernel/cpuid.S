.globl _cpuid
.text
	_cpuid:
		movl %ebx, %esi
		movl (%rdi), %eax
		movl 8(%rdi), %ecx
		cpuid
		movl %eax, (%rdi)
		movl %ebx, 4(%rdi)
		movl %ecx, 8(%rdi)
		movl %edx, 12(%rdi)
		movl %esi, %ebx
		ret
