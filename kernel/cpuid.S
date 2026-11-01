/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

.globl _cpuid
.text
	_cpuid:
		movl %ebx, %esi
		movl (%rdi), %eax
		movl 8(%rdi), %ecx
		cpuid
		movl %eax, (%rdi)
		movl %ebx, 4(%rdi)
		movl %ecx, 8(%rdi)
		movl %edx, 12(%rdi)
		movl %esi, %ebx
		ret
