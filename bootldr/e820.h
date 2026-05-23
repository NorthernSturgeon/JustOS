#ifndef __E820_H__
#define __E820_H__

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