.globl spin_lockb
.globl rwlock_inc
.globl rwlock_lock
.globl rwlock_inc_crit
.globl rwlock_lock_crit

.type rwlock_inc, STT_FUNC
.type rwlock_lock, STT_FUNC
.type rwlock_inc_crit, STT_FUNC
.type rwlock_lock_crit, STT_FUNC

.text
spin_lockb:
	endbr64
	mov $1, %cl
1:
	xor %al, %al
2:
	mov (%rdi), %dl
	pause
	test %dl, %dl
	jnz 2b
	lock cmpxchg %cl, (%rdi)
	jnz 1b
	ret

rwlock_inc:
	endbr64
	# i don't sure if xor instruction are needed
	xor %rcx, %rcx
1:
	xor %rax, %rax
	mov (%rdi), %eax
	pause
	mov %eax, %ecx
	inc %ecx
	lock cmpxchgq %rcx, (%rdi)
	jnz 1b
	xor %al, %al
	ret
rwlock_inc_crit:
	endbr64
	pushfq
	popq %rax
	test $0x200, %ax
	jz rwlock_inc
	# i don't sure if xor instruction are needed
	xor %rcx, %rcx
1:
	sti
	xor %rax, %rax
	mov (%rdi), %eax
	pause
	mov %eax, %ecx
	inc %ecx
	cli
	lock cmpxchgq %rcx, (%rdi)
	jnz 1b
	mov $1, %al
	ret
/*
rwlock_dec:
	endbr64
	lock decl (%rdi)
	ret
*/
rwlock_lock:
	endbr64
	movabs $0xFFFFFFFF00000000, %rcx
1:
	xor %rax, %rax
2:
	mov (%rdi), %rdx
	pause
	test %rdx, %rdx
	jnz 2b
	lock cmpxchgq %rcx, (%rdi)
	jnz 1b
	xor %al, %al
	ret
rwlock_lock_crit:
	endbr64
	pushfq
	popq %rax
	test $0x200, %ax
	jz rwlock_lock
	movabs $0xFFFFFFFF00000000, %rcx
1:
	sti
	xor %rax, %rax
2:
	mov (%rdi), %rdx
	pause
	test %rdx, %rdx
	jnz 2b
	cli
	lock cmpxchgq %rcx, (%rdi)
	jnz 1b
	mov $1, %al
	ret
