/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __E820_H__
#define __E820_H__

/**
 * \file e820.h
 */

typedef struct __attribute__((__packed__)){
	UINT64 base;
	UINT64 lenght;
	UINT32 type;
	UINT32 attr;
} e820_entry_t;

#define E820_TYPE_USABLE 1
#define E820_TYPE_RESERVED 2
#define E820_TYPE_ACPI_RECLAIM 3
#define E820_TYPE_ACPI_NVS 4
#define E820_TYPE_UNUSABLE 5

#define E820_ATTR_ACPI30_NV 0x2

#endif