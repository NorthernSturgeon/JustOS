.global start_kernel

.text/*
	page_fault_asm:
		popq %rdi
		mov %cr2, %rsi
		jmp page_fault*/
	start_kernel:
		mov %rcx, %cr3 # bye, efi paging
		# enable SSE support
		mov %cr0, %rax
		mov $0, %rcx
		not %rcx
		xor $0b1100, %rcx
		and %rcx, %rax
		or $2, %rax
		mov %rax, %cr0
		mov %cr4, %rax
		mov $0, %rcx
		not %rcx
		xor $0b10000000000, %rcx
		and %rcx, %rax
		or $0b1000000000, %rax
		mov %rax, %cr4
		sub $4, %rdx
		stmxcsr (%rdx)
		orl $0b1111111110000000, (%rdx)
		movl (%rdx), %ecx
		ldmxcsr (%rdx)
		add $4, %rdx
		mov %rdx, %rsp

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
		mov %rsp, %rax
		pushq %rax
		pushw $40
		mov %rsp, %rax
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