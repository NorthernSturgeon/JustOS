/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

.globl boot_info
.globl kentry
.globl __tls_get_addr
.globl asm_hlt

.text
	__boot_info:
		.rept 16
		.quad 0
		.endr
	__tls_get_addr:
		movq (%rdi), %rax
		movq %gs:(%rax), %rax
		ret
	kentry:
		and $-0x10, %rsp
		movq boot_info@GOTPCREL(%rip), %rcx
		movq %rdi, (%rcx)

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
