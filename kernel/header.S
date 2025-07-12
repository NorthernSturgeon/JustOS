.text
	.global _asm_ijmp

	_asm_ijmp:
		jmp .
	_pre_isr:
		movq $0, (%rsp)
		addq $4, %rsp
	_asm_iret:
		iretq

