.global start_kernel

.text
	start_kernel:
		andl $0xFFFFFFFF, %ecx
		movq %rcx, %cr3
		movq %rdx, %rsp
		# fill GDT
#		subq $40, %rsp
#		movq $0, (%rsp)
#		movq $0x00A09A000000FFFF, 8(%rsp)
#		movb $0, 12(%rsp)
#		movb $0x9A, 13(%rsp)
#		movb $0xA0, 14(%rsp)
#		movb $0, 15(%rsp)
#		movl $0x00A09A00, 12(%rsp)
#		movq $0x00C092000000FFFF, 16(%rsp)
#		movq $0x00A0FA000000FFFF, 24(%rsp)
#		movq $0x00C0F2000000FFFF, 32(%rsp)
		movabs $0x00C0F2000000FFFF, %rax
		pushq %rax
		movabs $0x00A0FA000000FFFF, %rax
		pushq %rax
		movabs $0x00C092000000FFFF, %rax
		pushq %rax
		movabs $0x00A09A000000FFFF, %rax
		pushq %rax
		xor %rax, %rax
		pushq %rax
		# setup GDT
		movq %rsp, %rax
		pushq %rax
		pushw $40
		movq %rsp, %rax
		lgdt (%rax)
		# reload GDT
		movw $0x10, %ax
		movw %ax, %ds
		movw %ax, %es
		movw %ax, %fs
		movw %ax, %gs
		movw %ax, %ss
		pushq $8
		pushq %rsi
		lretq 