.globl spin_lockb
.globl rwlock_inc
.globl rwlock_lock

.text
	spin_lockb:
		endbr64
		mov $1, %cl
	1:
		xor %al, %al
	2:
		mov (%rdi), %dx
		pause
		test %dx, %dx
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
		ret
