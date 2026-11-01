/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

.globl _rdmsr
.globl _wrmsr

.text
	_rdmsr:
		endbr64
		movl %edi, %ecx
		rdmsr
		shl $32, %rdx
		orq %rax, %rdx
		ret

	_wrmsr:
		endbr64
		movl %edi, %ecx
		movl %esi, %eax
		movq %rsi, %rdx
		shr $32, %rdx
		wrmsr
		ret