.globl boot_info
.globl kentry

.text
	__boot_info:
	.rept 11
	.quad 0
	.endr
	.rept 4
	.word 0
	.endr
	kentry:
	and $-0x10, %rsp
	lea __boot_info(%rip), %rax
	mov %rax, boot_info(%rip)
	jmp kmain

.data
	boot_info:
	.quad 0