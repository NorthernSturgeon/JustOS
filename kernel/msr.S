.globl _rdmsr
.globl _wrmsr

.text
	_rdmsr:
		endbr64
		movl %edi, %ecx
		rdmsr
		shl $32, %rdx
		orq %rax, %rdx
		ret

	_wrmsr:
		endbr64
		movl %edi, %ecx
		movl %esi, %eax
		movq %rsi, %rdx
		shr $32, %rdx
		wrmsr
		ret