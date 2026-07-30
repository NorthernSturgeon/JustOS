.globl boot_info
.globl kentry
.globl asm_hlt

.text
	__boot_info:
		.rept 14
		.quad 0
		.endr
	kentry:
		and $-0x10, %rsp
		lea __boot_info(%rip), %rax
		mov %rax, boot_info(%rip)

		mov $0xC0000101, %ecx
		xor %edx, %edx
		not %edx
		mov $-8, %eax
		wrmsr

		jmp kmain
	asm_hlt:
		hlt
		jmp asm_hlt
.data
	boot_info:
		.quad 0
