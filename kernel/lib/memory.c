#include "lib/types.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "lib/console.h"
#include "lib/atomic.h"

//(EFI_MEMORY_DESCRIPTOR*)((UINT8*)mmap+descsize*i);

gmrtp_t volatile gmrtp;

static inline size_t div_up(size_t a, size_t b) { return (a+b-1)/b; }

void modify_region(memory_region_t *region){

}

//void create_region(memory_region_t *region)
//void delete_region(memory_region_t *region)
//void* allocate_pages(size_t num, memory_region_type type, uint32_t attrib)
//void* free_pages(void* ptr)
//#define calloc(num, size) memset(malloc(num*size))

/*
static inline uint32_t efimemtypeconvert(EFI_MEMORY_TYPE memtype){
	switch (memtype){
	case EfiReservedMemoryType:
		return RegionReservedMemory;
	case EfiRuntimeServicesCode:
		return RegionFirmwareCode;
	case EfiRuntimeServicesData:
		return RegionFirmwareData;
	case EfiConventionalMemory:
		return RegionAvailableMemory;
	case EfiUnusableMemory:
		return RegionUnusableMemory;
	case EfiACPIReclaimMemory:
		return RegionACPIReclaimMemory;
	case EfiACPIMemoryNVS:
		return RegionACPINVS;
	case EfiMemoryMappedIO:
		return RegionMMIO;
	case EfiMemoryMappedIOPortSpace:
		return RegionMMIOPS;
	case EfiPalCode:
		return RegionPalCode;
	default:
		return RegionReservedMemory;
	}
}
*/

void init_mm(uint32_t bitmap, size_t bitmap_size){

}