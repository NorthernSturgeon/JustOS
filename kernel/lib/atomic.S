.globl spin_lockb

.text
    spin_lockb:
        endbr64
        mov $1, %cl
    loopb1:
        mov $0, %al
    loopb2:
        mov (%rdi), %dx
        pause
        test %dx, %dx
        jnz loopb2
        lock cmpxchg %cl, (%rdi)
        jnz loopb1
        ret