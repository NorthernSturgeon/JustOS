/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

#define bswap64(val) asm ("bswap %0\n":"+r"(val));
#define bswap32(val) asm ("bswap %0\n":"+r"(val));
#define bswap16(val) asm ("xchg %%al, %%ah":"+a"(val));

#endif