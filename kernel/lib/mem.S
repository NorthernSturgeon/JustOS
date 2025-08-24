.globl memcpy
.globl memset
.globl memmove

.text
	memcpy:
		endbr64
		movq %rdi, %rax
		test %rdi, %rsi
		cld
		jne memmain
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
		std
	memmain:
		cmp $8, %rdx
		jb memmain3 #count < 8
		movq %rdi, %r8
		and $7, %r8
		jz memmain2 #dest % 8 == 0
		sub $8, %r8
		sub %r8, %rdx
		lea memmain2(%rip), %rcx
		sub %r8, %rcx
		jmp *%rcx
		movsb
		movsb
		movsb
		movsb
		movsb
		movsb
		movsb
	memmain2:
		movq %rdx, %rcx
		shr $3, %rcx
		rep movsq
	memmain3:
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
