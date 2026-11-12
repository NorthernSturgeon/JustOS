# Interrupts subsystem {#int}
> We made it: the vector number is now in our hands!

### Stages of primary interrupt handling

#### 1. Entry point
> Each possible interrupt vector has its own unique entry point that pushes the vector number onto the stack and then transfers control to stage 2 (isr.S). Since we cannot use registers without first saving them, we are forced to push the number onto the stack. A recursive macro with an incrementing parameter is used to create the 256 distinct entry points. Note that the push instruction is encoded directly via its opcode; otherwise, GAS generates different instructions for vectors numbered 0-127 versus 128-255, which breaks the uniformity of instruction length and operand size on the stack.  
>  
> -- Description of file "isr_id.S"

#### 2. Wrapper
Small code in assembler ("isr.S"), which extracts the interrupt vector, saves the context and transfers control to the high-level handler. On return, restores the context and recovers the stack.

#### 3. High-level code
This stage checks if the interrupt has the error code and calls the specific handler for the interrupt vector.