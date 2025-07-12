.globl _memcpy
.globl _memset
.globl _memmove

.text
	_memmove:
		endbr64
		movq %rdi, %rax
		test %rdi, %rsi
		je memret
		add %rdx, %rsi
		cmp %rsi, %rdi
		jb memmove2
		sub %rdx, %rsi
		cld
		jmp memmain
	memmove2:
		add %rdx, %rdi
		std
	memmain:
		movq %rdx, %rcx
		shr $3, %rcx
		rep movsq
		and $7, %rdx
		jz memret
		lea memret(%rip), %rcx
		sub %rdx, %rcx
		jmp *%rcx
		movsb
		movsb
		movsb
		movsb
		movsb
		movsb
		movsb
	memret:
		cld
		ret
	_memcpy:
		endbr64
		movq %rdi, %rax
		test %rdi, %rsi
		je memret
		cld
		jmp memmain
	_memset:
		endbr64
		test %rdx, %rdx
		jz memset2
		cld
		movq %rsi, %rax
		movq %rdx, %rcx
		rep stosb
	memset2:
		ret
