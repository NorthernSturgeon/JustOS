#ifndef __BL_MAIN_H__
#define __BL_MAIN_H__

/**
 * \file bootloader.h
 */

#include <efi/efi.h>
#include <efi/efilib.h>

//#define ErrorCheck(actual, expected, msg) if(actual != expected) {Print(msg);return actual;}

//#define CONV_PTR(p) ((VOID*)(((UINT64)(p))|(0xffff8ull<<44)))
//                                          6456484032241608
#define CONV_PTR(p) (void*)(((UINT64)(p))|0xffff800000000000ull)

#define STOP() for(;;)

/**
 Setup EfiRuntimeServices support
 If commented, boot_info->rtsvcs will be set to zero
*/
//#define SETVAMAP

/**
 Enables the workaround for a bug in the UEFI firmware
 that causes a system crash when an attempt is made
 to assign a memory type from a range
 reserved for use by OS developers.
*/
#define ovmf_r11337

/** GOP video mode overriding */
//#define CUSTOM_VIDEOMODE 12

/**
 JustOS makes an assumption that the system has less than 2^48
 bytes of RAM; otherwise, address translation will be wrong.
*/
#define MAX_PHY_ADDR 0x00007fffffffffffull

#ifdef ovmf_r11337
#define KERNEL_MEMORY_TYPE EfiLoaderData
#else
#define KERNEL_MEMORY_TYPE 0x80000000
#endif

/** Memory allocated via these macroses will be preserved in the kernel */
#define ALLOC_KPOOL(ptr, size) uefi_call_wrapper(BS->AllocatePool, 3, KERNEL_MEMORY_TYPE, size, ptr)
#define FREE_KPOOL(ptr) uefi_call_wrapper(BS->FreePool, 1, ptr)
#define ALLOC_KPAGES(ptr, size) uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, KERNEL_MEMORY_TYPE, size, ptr)
#define FREE_KPAGES(ptr, size) uefi_call_wrapper(BS->FreePages, 2, ptr, size)

static inline size_t div_up(size_t a, size_t b) { return (a+b-1)/b; }
#define TO_PAGES(ptr) div_up(ptr, 4096)

/** Macro for indexing into the EfiMemoryMap, which is pointed to by `mmap` variable */
#define MMAP(i) ((EFI_MEMORY_DESCRIPTOR*)((UINT8*)mmap+descsize*i))

#define VALID_DESC(desc) ((desc->Type<EfiMaxMemoryType || desc->Type==KERNEL_MEMORY_TYPE) && desc->NumberOfPages && !(desc->PhysicalStart&4095))
#define PTZONEMEM(desc) ((desc->Type==EfiBootServicesData)||(desc->Type==EfiBootServicesCode)||(desc->Type==EfiConventionalMemory))

//                     6448403224160800
#define PTENTRY_MASK 0x0000fffffffff000ull

#define DFLT_PTE_ATTR 3 // WRITE | PRESENT
#define GET_PTE(addr) ((page_entry_t*)(((UINT64)(addr))&PTENTRY_MASK))
#define SET_PTE(addr, attr) ((((UINT64)(addr))&PTENTRY_MASK)|(attr))

extern __attribute__((noreturn)) VOID Fatality(UINT32 code, UINT32 subcode);
extern UINTN ReadFile(EFI_FILE_HANDLE fileDir, CHAR16 *filename, VOID** buffer, UINTN addend);
extern VOID FileDestructor(VOID* file, UINTN filesz);

#endif