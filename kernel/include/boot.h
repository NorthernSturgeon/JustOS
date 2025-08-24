#ifndef __BOOTINFO_H__
#define __BOOTINFO_H__

typedef struct __packed{
	rtsvcs_t *rtsvcs;
	void *acpi_rdsp;
	void *ptzone;
	uint64_t ptzone_size;
	void *bitmap;
	uint64_t bitmap_size;
	void *vram;
	uint64_t vram_size;
	uint16_t width;
	uint16_t height;
	uint16_t format;
	uint16_t ppl;
} boot_info_t;

extern const boot_info_t boot_info;

#endif