.globl chbo16
.globl chbo32
.globl chbo64

.text
	chbo16:
		endbr64
		movw %di, %ax
		rol $8, %ax
		ret

	chbo32:
		endbr64
		movl %edi, %eax
		bswap %eax
		ret

	chbo64:
		endbr64
		movq %rdi, %rax
		bswap %rax
		ret
