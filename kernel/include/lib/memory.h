/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __MEMORY_H__
#define __MEMORY_H__

extern void* malloc(size_t size);
extern void free(void* ptr);

extern uint64_t round_to(uint64_t n, uint64_t k);

#endif