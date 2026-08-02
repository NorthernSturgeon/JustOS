/**
 *	isr.S - 2nd stage of universal interrupt handler
 * 	targets:
 *		1) save context, extract vector number and transfer control to high-level handler
 * 		2) restore context and stack, return control to interrupted code
 *
 * The code is thoroughly commented, so I have nothing to add here...
 */
.globl isr_common_stub
.extern isr_common

.type isr_common_stub, STT_FUNC

.text
isr_common_stub:
	# save caller-saved GPR
	push %rax
	push %rdi
	push %rsi
	push %rdx
	push %rcx
	push %r8
	push %r9
	push %r10
	push %r11

	# extract int vector
	movb 72(%rsp), %dil

	# get pointer to stack at int
	leaq 80(%rsp), %rsi

	# we need register for stack operations
	push %r12

	# reserve place for fxsave
	sub $512, %rsp

	# realign stack to 16 bytes and record how many bytes we borrow
	mov %rsp, %r12
	and $0x0f, %r12
	sub %r12, %rsp

	# save MMX/SSE state
	fxsave64 (%rsp)

	# call high-level handler
	call isr_common

	# restore MMX/SSE
	fxrstor64 (%rsp)

	# return stack space, including the borrowed bytes
	add %r12, %rsp
	add $512, %rsp

	# check if we need to pop error code
	test %al, %al

	# restore GPR (pop instruction does not affect the flags)
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rcx
	pop %rdx
	pop %rsi
	pop %rdi
	pop %rax

	# if zero - there is no error code on stack
	jz 1f
	# pop it otherwise
	add $8, %rsp
1:
	# pop int vector
	add $8, %rsp

	# fin
	iretq
