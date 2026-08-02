/**
 *	isr_id.S - 1st stage of universal interrupt handler
 *	target - determine the interrupt vector
 *
 * Each possible interrupt vector has its own unique entry point that pushes the
 * vector number onto the stack and then transfers control to stage 2 (isr.S).Since we
 * cannot use registers without first saving them, we are forced to push the number
 * onto the stack. A recursive macro with an incrementing parameter is used to
 * create the 256 distinct entry points. Note that the instruction is encoded
 * directly via its opcode; otherwise, GAS generates different instructions for
 * vectors numbered 0–127 versus 128–255, which breaks the uniformity of instruction
 * length and operand size on the stack.
 */
.globl isr_start
.extern isr_common_stub

.macro generator from, to
	.byte 0x6a # push imm8 (2 bytes)
	.byte \from
	jmp isr_common_stub # jmp rel32 (5 bytes)
	# total: 7 bytes per entry point
.if \to-\from
	generator "(\from+1)",\to
.endif
.endm

.type isr_start, STT_FUNC

.text
isr_start:
	# bypassing recursion limit
	generator 0,63
	generator 64,127
	generator 128,191
	generator 192,255