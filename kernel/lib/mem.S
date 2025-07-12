.globl memcpy
.globl memset
.globl memmove

.text
	memmove:
		endbr64
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
		and $7, %rdx
		rep movsq
		mov %rdx, %rcx
		rep movsb
	memret:
		cld
		ret
	memcpy:
		endbr64
		test %rdi, %rsi
		je memret
		cld
		jmp memmain
	memset:
		endbr64
		test %rdx, %rdx
		jz memret
	memset2:
		movb %sil, (%rdi)
		incq %rdi
		decq %rdx
		jnz memset2
		
