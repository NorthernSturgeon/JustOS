/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

.global start_kernel

.text
	start_kernel:
		mov %rcx, %cr3 # bye, efi paging

		# enable SSE support
		mov %cr0, %rax

		xor %rcx, %rcx
		not %rcx
		xor $0b1100, %rcx

		and %rcx, %rax
		or $2, %rax
		mov %rax, %cr0
		mov %cr4, %rax

		xor %rcx, %rcx
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

		# set flags.nt to 0
		pushfw
		andw $0xBFFF, (%rsp)
		popfw

		# fill GDT
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
		movw %ax, %ss
		xorw %ax, %ax
		movw %ax, %fs
		movw %ax, %gs

		pushq $8
		pushq %rsi
		lretq 