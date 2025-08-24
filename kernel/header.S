.globl boot_info
.globl kentry

.text
	boot_info:
	.rept 8
	.quad 0
	.endr
	.rept 4
	.word 0
	.endr
	kentry:
	and $-0x10, %rsp
	jmp kmain