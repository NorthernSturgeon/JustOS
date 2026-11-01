/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __CPUID_H__
#define __CPUID_H__

typedef struct{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
} cpuid_t;

static __always_inline cpuid_t __cpuid(uint32_t eax, uint32_t ecx) {
	cpuid_t values = {
		.eax = eax,
		.ecx = ecx
	};

	asm ("cpuid;":"+a"(values.eax), "=b"(values.ebx), "+c"(values.ecx), "=d"(values.edx):);

	return values;
}

#endif