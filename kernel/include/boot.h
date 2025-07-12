#ifndef __BOOTINFO_H__
#define __BOOTINFO_H__

typedef struct __packed{
	void *kernel;
	rtsvcs_t *rtsvcs;
	void *acpi_rdsp;
	void *ptzone;
	uint64_t ptzone_size;
	void *desc_list;
	uint64_t desc_size;
	void *vram;
	uint64_t vram_size;
	uint16_t width;
	uint16_t height;
	uint16_t format;
	uint16_t ppl;
} boot_info_t;

#endif