#ifndef __PHYSMEM_H__
#define __PHYSMEM_H__

#include <stdatomic.h>

//                        BADPOINTERBADPTR
#define BAD_POINTER     0xBAD90187E2BAD972
#define EFI_BAD_POINTER 0xFBFBFBFBFBFBFBFB
#define memmask         0xFFFF800000000000ull

#define phys_to_virt(ptr) ((void*)((uint64_t)(ptr)|memmask))
#define virt_to_phys(ptr) ((void*)((uint64_t)(ptr)&~memmask))
 
#ifndef __E820_H__
#define __E820_H__

typedef struct __packed{
	uint64_t base;
	uint64_t lenght;
	uint32_t type;
	uint32_t attr;
} e820_entry_t;

#define E820_TYPE_USABLE 1
#define E820_TYPE_RESERVED 2
#define E820_TYPE_ACPI_RECLAIM 3
#define E820_TYPE_ACPI_NVS 4
#define E820_TYPE_UNUSABLE 5

#define E820_ATTR_ACPI30_NV 0x2

#endif

extern e820_entry_t *e820;
extern size_t e820_len;

//global occupied region list
typedef struct{
	uint64_t *list; //atomic
	uint64_t lenght;
	uint64_t rwlock;
} gorl_t;

extern gorl_t gorl;

extern void init_mm();
extern void* allocate_pages(size_t size);
extern void free_pages(void* ptr, size_t size);

#endif