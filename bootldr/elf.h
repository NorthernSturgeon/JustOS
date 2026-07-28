#ifndef __BL_ELF_H__
#define __BL_ELF_H__

#define EI_MAG 0x464c457f //little-endian

#include <linux/elf.h>

#define R_X86_64_NONE		0
#define R_X86_64_GLOB_DAT	6
#define R_X86_64_JUMP_SLOT	7
#define R_X86_64_RELATIVE	8

extern uint8_t check_elf(void* file);
extern Elf64_Shdr *get_section_by_name(Elf64_Ehdr* image, const char* name);
extern Elf64_Dyn* get_dyn(Elf64_Ehdr* image);
extern Elf64_Dyn* get_dynent(Elf64_Dyn* start, Elf64_Sxword tag);
extern void* off2ptr(void* base, Elf64_Dyn* ent); //return ent ? base + ent->d_un.d_val : NULL;

#endif