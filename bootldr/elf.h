/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __BL_ELF_H__
#define __BL_ELF_H__

/**
 * \file elf.h
 */

#define EI_MAG 0x464c457f //little-endian

#include <linux/elf.h>

#define R_X86_64_NONE		0
#define R_X86_64_GLOB_DAT	6
#define R_X86_64_JUMP_SLOT	7
#define R_X86_64_RELATIVE	8
#define R_X86_64_DTPMOD64	16

extern uint8_t check_elf(void* file);
extern Elf64_Shdr *get_section_by_name(Elf64_Ehdr* image, const char* name);
extern Elf64_Phdr *get_segment_by_type(Elf64_Ehdr* image, Elf64_Word type);
extern Elf64_Dyn* get_dyn(Elf64_Ehdr* image);
extern Elf64_Dyn* get_dynent(Elf64_Dyn* start, Elf64_Sxword tag);
extern void* off2ptr(void* base, Elf64_Dyn* ent); //return ent ? base + ent->d_un.d_val : NULL;

#endif