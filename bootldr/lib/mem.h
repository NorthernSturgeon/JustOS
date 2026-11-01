/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

//future funcionality

#ifndef __MEM_H__
#define __MEM_H__

extern void _memcpy(void* dest, void* src, size_t size);
extern void _memmove(void* dest, void* src, size_t size);
extern void _memset(void* dest, uint8_t c, size_t size); 

#endif