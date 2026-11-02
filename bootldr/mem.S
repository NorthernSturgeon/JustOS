/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

# from kernel/lib/mem.S

.globl _memcpy
.globl _memset
.globl _memmove

.text
_memcpy:
	endbr64
	movq %rdi, %rax
	test %rdi, %rsi
	cld
	jne memmain
memret2:
	ret
_memmove:
	endbr64
	movq %rdi, %rax
	test %rdi, %rsi
	je memret
	cmp %rsi, %rdi
	ja memmove2
	cld
	jmp memmain
memmove2:
	add %rdx, %rsi
	add %rdx, %rdi
	dec %rsi
	dec %rdi
	std
_memmain:
	movq %rdx, %rcx
	rep movsb
	cld
memret:
	ret
_memset:
	endbr64
	movq %rdi, %rax
	test %rdi, %rdi
	jz memset2
	test %rdx, %rdx
	jz memset2
	cld
	movq %rsi, %rax
	movq %rdx, %rcx
	rep stosb
memset2:
	ret
