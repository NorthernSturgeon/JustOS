.globl memcpy
.globl memset
.globl memmove

.type memcpy, STT_FUNC
.type memset, STT_FUNC
.type memmove, STT_FUNC

.text
	memcpy:
		endbr64
		movq %rdi, %rax
		test %rdi, %rsi
		cld
		jne memmain
	memret2:
		ret
	memmove:
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
		dec %rsi
		dec %rdi
		std
	memmain:
		movq %rdx, %rcx
		rep movsb
		cld
	memret:
		ret
	memset:
		endbr64
		movq %rdi, %rax
		test %rdi, %rdi
		jz memset2
		test %rdx, %rdx
		jz memset2
		cld
		movq %rsi, %rax
		movq %rdx, %rcx
		rep stosb
	memset2:
		ret
