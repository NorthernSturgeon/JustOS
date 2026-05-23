#ifndef __BOOTINFO_H__
#define __BOOTINFO_H__

#include "rtsvcs.h"

typedef struct __packed{
	rtsvcs_t *rtsvcs; // 1
	void *acpi_rdsp; // 2
	void *stack; // 3
	uint64_t stack_size; // 4
	uint64_t kernel_size; // 5
	void *ptzone; // 6
	uint64_t ptzone_size; // 7
	e820_entry_t *mmap; // 8
	uint64_t mmap_len; // 9
	void *vram; // 10
	uint64_t vram_size; // 11
	uint16_t width;
	uint16_t height;
	uint16_t format;
	uint16_t ppl;
} boot_info_t;

extern const boot_info_t *boot_info;

#endif