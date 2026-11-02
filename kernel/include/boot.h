/*
 *	Copyright (c) 2026 NorthernSturgeon
 *	SPDX-License-Identifier: MPL-2.0
 *
 *	This Source Code Form is subject to the terms of the Mozilla Public 
 *	License, v. 2.0. If a copy of the MPL was not distributed with this
 *	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __BOOTINFO_H__
#define __BOOTINFO_H__

#include "physmem.h"

typedef struct __packed{
	char* name;
	void* data;
	size_t size;
	void* tls_area;
	size_t tls_size;
} loaded_file;

typedef struct __packed{
	void *rtsvcs; // 0
	void *acpi_rdsp; // 1
	void *stack; // 2
	uint64_t stack_size; // 3
	uint64_t kernel_size; // 4
	void *ptzone; // 5
	uint64_t ptzone_size; // 6
	e820_entry_t *mmap; // 7
	uint64_t mmap_len; // 8
	loaded_file *files; //9
	size_t files_cnt; //10
	void *splash; //11
	size_t splash_size; //12
	void *vram; // 13
	uint64_t vram_size; // 14
	uint16_t width; //15
	uint16_t height;
	uint16_t format;
	uint16_t ppl;
	//16
} boot_info_t;

extern const boot_info_t *boot_info;

#endif