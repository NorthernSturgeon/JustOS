#include <efi/efi.h>
#include <efi/efilib.h>

#include "serial.h"
#include "elf.h"
#include "ldrerr.h"
#include "lib/video.h"
#include "lib/console.h"
#include "e820.h"
#include "tsv.h"

//#define ErrorCheck(actual, expected, msg) if(actual != expected) {Print(msg);return actual;}

//#define CONV_PTR(p) ((VOID*)(((UINT64)(p))|(0xffff8ull<<44)))
//                                          6456484032241608
#define CONV_PTR(p) (void*)(((UINT64)(p))|0xffff800000000000ull)

#define STOP() for(;;)

typedef union{
	UINT64 addr;
	struct{
		UINT64 offset:12;
		UINT64 pt:9;
		UINT64 pd:9;
		UINT64 pdt:9;
		UINT64 pml4:9;
		UINT64 rsv:16;
	} f;
} virt_addr_t;

typedef union{
	UINT64 value;
	struct{
		UINT64 p:1;
		UINT64 rw:1;
		UINT64 us:1;
		UINT64 pwt:1;
		UINT64 pcd:1;
		UINT64 a:1;
		UINT64 d:1;
		UINT64 pat:1;
		UINT64 g:1;
		UINT64 ign:2;
		UINT64 r:1;
		UINT64 addr:36;
		UINT64 rsv:15;
		UINT64 xd:1;
	} f;
} page_entry_t;

typedef struct __attribute__((__packed__)){
	VOID *rtsvcs;
	VOID *acpi_rdsp;
	page_entry_t *ptzone;
	UINT64 ptzone_size;
	e820_entry_t *mmap;
	UINT64 mmap_len;
	VOID *vram;
	UINT64 vram_size;
	UINT16 width;
	UINT16 height;
	UINT16 format;
	UINT16 ppl;
} boot_info_t;

/*
typedef struct __attribute__((__packed__)){
   uint16_t offset_1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_2;        // offset bits 16..31
   uint32_t offset_3;        // offset bits 32..63
   uint32_t zero;            // reserved
} idt_entry_t;

typedef struct __attribute__((__packed__)){
	uint16_t limit;
	void    *base;
} idtr_t;

idtr_t itdr;
*/
#undef DEBUG
//#define SETVAMAP
#define ovmf_r11337
//#define CUSTOM_VIDEOMODE 12

#define STACK_SIZE 32768
//                     6347393123150700
#define MAX_PHY_ADDR 0x00007fffffffffffull

#ifdef ovmf_r11337
#define KERNEL_MEMORY_TYPE EfiLoaderData
#else
#define KERNEL_MEMORY_TYPE 0x80000000
#endif

#define ALLOC_KPOOL(ptr, size) uefi_call_wrapper(BS->AllocatePool, 3, KERNEL_MEMORY_TYPE, size, ptr)
#define FREE_KPOOL(ptr) uefi_call_wrapper(BS->FreePool, 1, ptr)
#define ALLOC_KPAGES(ptr, size) uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, KERNEL_MEMORY_TYPE, size, ptr)
#define FREE_KPAGES(ptr, size) uefi_call_wrapper(BS->FreePages, 2, ptr, size)

static inline size_t div_up(size_t a, size_t b) { return (a+b-1)/b; }
#define TO_PAGES(ptr) div_up(ptr, 4096)

#define MMAP(i) ((EFI_MEMORY_DESCRIPTOR*)((UINT8*)mmap+descsize*i))

#define VALID_DESC(desc) ((desc->Type<EfiMaxMemoryType || desc->Type==KERNEL_MEMORY_TYPE) && desc->NumberOfPages && !(desc->PhysicalStart&4095))
#define PTZONEMEM(desc) ((desc->Type==EfiBootServicesData)||(desc->Type==EfiBootServicesCode)||(desc->Type==EfiConventionalMemory))

//                     6448403224160800
#define PTENTRY_MASK 0x0000fffffffff000ull

#define GET_PTE(addr) ((page_entry_t*)((UINT64)(addr)&PTENTRY_MASK))
#define SET_PTE(addr, attr) ((((UINT64)(addr))&PTENTRY_MASK)|attr)

extern VOID start_kernel(boot_info_t *boot_info, VOID *k_entry, VOID *k_stack, VOID *cr3);

//extern VOID page_fault_asm();

#ifdef DEBUG
static const WCHAR* memtypeconvert(EFI_MEMORY_TYPE memtype){
	switch (memtype){
	case EfiReservedMemoryType:
		return L"reserved";
	case EfiLoaderCode:
		return L"ldr-code";
	case EfiLoaderData:
		return L"ldr-data";
	case EfiBootServicesCode:
		return L"BS-code ";
	case EfiBootServicesData:
		return L"BS-data ";
	case EfiRuntimeServicesCode:
		return L"RT-code ";
	case EfiRuntimeServicesData:
		return L"RT-data ";
	case EfiConventionalMemory:
		return L"conv-mem";
	case EfiUnusableMemory:
		return L"unusable";
	case EfiACPIReclaimMemory:
		return L"ACPIrecl";
	case EfiACPIMemoryNVS:
		return L"ACPI-NVS";
	case EfiMemoryMappedIO:
		return L"MMIO    ";
	case EfiMemoryMappedIOPortSpace:
		return L"MMIO-PS ";
	case EfiPalCode:
		return L"palcode ";
	default:
		return L"unknown ";
	}
}
#endif

static VOID Fatality(UINT32 code, UINT32 subcode){
	Print(L"\nFatality! %u:%u\n", code, subcode);
	UINTN KeyEvent = 0;
	EFI_INPUT_KEY Key;
	Print(L"Press any key to reboot...\n");
	uefi_call_wrapper(ST->ConIn->Reset, 2, ST->ConIn, FALSE);
	uefi_call_wrapper(BS->WaitForEvent, 3, 1, &ST->ConIn->WaitForKey, &KeyEvent);
	uefi_call_wrapper(ST->ConIn->ReadKeyStroke, 2, ST->ConIn, &Key);
	Print(L"RT->ResetSystem(EfiResetCold);");
	uefi_call_wrapper(RT->ResetSystem,4,EfiResetCold,EFI_SUCCESS,0,NULL);
}

static VOID GetMMap(EFI_MEMORY_DESCRIPTOR **mmap, UINTN *mmapsize,
	UINTN *mapkey, UINTN *descsize, 
	UINT32 *descver, UINTN *num_entries){
	register EFI_STATUS status = uefi_call_wrapper(BS->GetMemoryMap, 5, mmapsize, *mmap, mapkey, descsize, descver);
	*mmapsize += 2*(*descsize);
	uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, TO_PAGES(*mmapsize), mmap);
	status = uefi_call_wrapper(BS->GetMemoryMap, 5, mmapsize, *mmap, mapkey, descsize, descver);
	while (status!=EFI_SUCCESS){
		//Print(L"Attempt %u: Size=%u\n", i, *mmapsize);
		uefi_call_wrapper(BS->FreePages, 2, TO_PAGES(*mmapsize), mmap);
		*mmapsize += 2*(*descsize);
		uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, TO_PAGES(*mmapsize), mmap);
		status = uefi_call_wrapper(BS->GetMemoryMap, 5, mmapsize, *mmap, mapkey, descsize, descver);
	}
	*num_entries = *mmapsize / *descsize;
	//Print(L"Success! Size=%u\n", *mmapsize);
}

static EFI_STATUS OpenRoot(EFI_HANDLE imgHand, EFI_FILE_HANDLE *file){
	EFI_LOADED_IMAGE_PROTOCOL* loadedImg;
	uefi_call_wrapper(BS->HandleProtocol, 3, imgHand, &gEfiLoadedImageProtocolGuid, (void**)&loadedImg);
	
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* filesystem;
	uefi_call_wrapper(BS->HandleProtocol, 3, loadedImg->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&filesystem);

	return uefi_call_wrapper(filesystem->OpenVolume, 2, filesystem, file);
}

UINTN ReadFile(EFI_FILE_HANDLE fileDir, CHAR16 *filename, VOID** buffer, UINTN addend){
	EFI_FILE_HANDLE file;
	register EFI_STATUS status = uefi_call_wrapper(fileDir->Open, 5, fileDir, &file, filename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (EFI_ERROR(status)) Fatality(LDR_FILE, status);

	EFI_FILE_INFO* fileinfo = LibFileInfo(file);
	UINTN filesz = TO_PAGES(fileinfo->FileSize)+addend;
	ALLOC_KPAGES(buffer, filesz);
	filesz <<= 12; // pages -> bytes

	status = uefi_call_wrapper(file->Read, 3, file, &filesz, *buffer);
	FreePool(fileinfo);
	if (EFI_ERROR(status)) Fatality(LDR_FILE, status);

	uefi_call_wrapper(file->Close, 1, file);

	return filesz;
}

static EFI_STATUS allocate_ptzone(boot_info_t *boot_info, EFI_MEMORY_DESCRIPTOR *mmap, UINTN num_entries, UINTN descsize){
	UINT64 totalsize = TO_PAGES(boot_info->vram_size);
	for(UINT64 cur_entry=0;cur_entry<num_entries;cur_entry++){
		EFI_MEMORY_DESCRIPTOR *desc = MMAP(cur_entry);
		if (!VALID_DESC(desc)) continue;
		if (desc->PhysicalStart == (EFI_PHYSICAL_ADDRESS)boot_info->vram) totalsize -= desc->NumberOfPages; //is VRAM described in efi memory map?
		totalsize += desc->NumberOfPages;
	}
	Print(L"Total memory size: %llu\n", totalsize<<12);
	//Print(L"PTZONE size: %llu\n", totalsize<<4);
	//Print(L"PTZONE size: %llu\n", totalsize>>8);
	boot_info->ptzone = (VOID*)0xFFFFFFFF;
	EFI_STATUS status = uefi_call_wrapper(BS->AllocatePages, 4, AllocateMaxAddress, KERNEL_MEMORY_TYPE, totalsize>>8, &boot_info->ptzone);
	if (status == EFI_OUT_OF_RESOURCES) {
		Print(L"Error: not enough memory!\n");
		STOP();
	} else if (EFI_ERROR(status)){
		Print(L"Error: unknown!\n");
		STOP();
	} else {
		Print(L"PTZONE: %p\n", boot_info->ptzone);
		Print(L"To clear in PTZONE: %llu\n", totalsize<<4);
		Print(L"Clear PTZONE... ");
		ZeroMem(boot_info->ptzone ,totalsize<<4);
		Print(L"success!\n");
		boot_info->ptzone_size = totalsize>>8;
	}
	return status;
}

static VOID convert_mmap_to_e820(boot_info_t *boot_info, EFI_MEMORY_DESCRIPTOR *mmap, UINTN num_entries, UINTN descsize){
	e820_entry_t *e820_table = (e820_entry_t*)(boot_info->ptzone + boot_info->ptzone_size*512);

	UINT64 bad_descs = 0;
	for (UINT64 i = 0; i < num_entries; i++){
		EFI_MEMORY_DESCRIPTOR *tempptr = MMAP(i);
		//is it bad descriptor?
		if (tempptr->PhysicalStart&0xFFF || tempptr->NumberOfPages == 0 \
		|| (tempptr->Type >= EfiMaxMemoryType && tempptr->Type < 0x70000000)){
			printf("efimmap: bad %p %u\r\n", tempptr->PhysicalStart, i);
			bad_descs++;
			continue;
		}
		e820_entry_t *cur_entry = e820_table+i-bad_descs;
		cur_entry->base = tempptr->PhysicalStart;
		cur_entry->lenght = tempptr->NumberOfPages<<12;
		cur_entry->attr = (UINT32)tempptr->Attribute;
		switch (tempptr->Type){
			case EfiConventionalMemory:
			case EfiLoaderCode:
			case EfiLoaderData:
			case EfiBootServicesCode:
			case EfiBootServicesData:
#ifndef SETVAMAP
			case EfiRuntimeServicesCode:
			case EfiRuntimeServicesData:
#endif
				cur_entry->type = E820_TYPE_USABLE;
				break;
			case EfiACPIReclaimMemory:
				cur_entry->type = E820_TYPE_ACPI_RECLAIM;
				break;
			case EfiACPIMemoryNVS:
				cur_entry->type = E820_TYPE_ACPI_NVS;
				break;
			case EfiUnusableMemory:
				cur_entry->type = E820_TYPE_UNUSABLE;
				break;
			default:
				cur_entry->type = E820_TYPE_RESERVED;
				break;
		}
	}
	boot_info->mmap = e820_table;
	boot_info->mmap_len = num_entries -  bad_descs;
}

static VOID alloc_page(boot_info_t *boot_info, virt_addr_t vaddr, UINT64 *free_ptzonesz, UINT16 pml4offset, uint64_t attr){
	page_entry_t *cur_pt, *prev_pt;
	cur_pt = GET_PTE(boot_info->ptzone[vaddr.f.pml4+pml4offset].value); //get pdt from pml4
	if (cur_pt == NULL) { //pdt doesn't exists
		cur_pt = boot_info->ptzone + (boot_info->ptzone_size - (*free_ptzonesz)--)*512; //create pdt
		boot_info->ptzone[vaddr.f.pml4+pml4offset].value = SET_PTE(cur_pt, 3); //set pdt in pml4
	}
	prev_pt = cur_pt; //save current pdt
	cur_pt = GET_PTE(cur_pt[vaddr.f.pdt].value); //get pd in pdt
	if (cur_pt == NULL) { //pd doesn't exists
		cur_pt = boot_info->ptzone + (boot_info->ptzone_size - (*free_ptzonesz)--)*512; //create pd
		prev_pt[vaddr.f.pdt].value = SET_PTE(cur_pt, 3); //set pd in pdt
	}
	prev_pt = cur_pt; //save current pd
	cur_pt = GET_PTE(cur_pt[vaddr.f.pd].value); //get pt from pd
	if (cur_pt == NULL) { //pt doesn't exists
		cur_pt = boot_info->ptzone + (boot_info->ptzone_size - (*free_ptzonesz)--)*512; //create pt
		prev_pt[vaddr.f.pd].value = SET_PTE(cur_pt, 3); //set pt in pd
	}
	cur_pt[vaddr.f.pt].value = SET_PTE(vaddr.addr, attr); //set page in pt
}

static VOID setup_paging(boot_info_t *boot_info, EFI_MEMORY_DESCRIPTOR *mmap, UINTN num_entries, UINTN descsize){
	UINT64 cur_entry;
	virt_addr_t vaddr;

	UINT64 free_ptzonesz=boot_info->ptzone_size-1; //first page of boot_info->ptzone is pml4
	printf("free_ptzonesz (bytes): %u\r\n", free_ptzonesz<<12);
	printf("start kernel at: %p\r\n", start_kernel); //cr3 reload in start_kernel, so it must be mapped in both higher and lower halves
	
	vaddr = (virt_addr_t)((UINT64)start_kernel&(MAX_ADDRESS<<12));
	printf("1st startkernel page: %p\r\n", vaddr.addr);
	printf("2nd startkernel page: %p\r\n", vaddr.addr+4096);
	
	alloc_page(boot_info, vaddr, &free_ptzonesz, 0, 3);
	vaddr.addr += 4096;
	alloc_page(boot_info, vaddr, &free_ptzonesz, 0, 3);
	
	for (vaddr.addr = (UINT64)boot_info->vram; \
	vaddr.addr < (UINT64)boot_info->vram + boot_info->vram_size; \
	vaddr.addr += 4096) {
		alloc_page(boot_info, vaddr, &free_ptzonesz, 256, 3);
	}

	for(cur_entry=0;cur_entry<num_entries;cur_entry++){
		EFI_MEMORY_DESCRIPTOR *desc = MMAP(cur_entry);
		if (!VALID_DESC(desc)) continue;
		if (desc->Attribute&EFI_MEMORY_RUNTIME) desc->VirtualStart = (EFI_VIRTUAL_ADDRESS)CONV_PTR(desc->PhysicalStart);

		UINT64 pte_flags = 0x3;
		if (desc->Type==EfiUnusableMemory || desc->Type == EfiReservedMemoryType) pte_flags &= ~1ull;
		if (desc->Type==EfiMemoryMappedIO || desc->Type == EfiMemoryMappedIOPortSpace) pte_flags |= 0x10;

		for (vaddr.addr=desc->PhysicalStart; vaddr.addr < desc->NumberOfPages*4096+desc->PhysicalStart; vaddr.addr+=4096){
			if (free_ptzonesz <= 3) goto error; //paging setup failed due to insufficient memory
			alloc_page(boot_info, vaddr, &free_ptzonesz, 256, pte_flags);
		}
	}

	boot_info->ptzone_size -= free_ptzonesz;
	printf("PTZONE true size: %u\r\n", boot_info->ptzone_size<<12);
	return;
	error:
	printf("setup_paging: FATAL: insufficient memory!\r\n");
	STOP();
	//uefi_call_wrapper(SystemTable->RuntimeServices,4,EfiResetShutdown,EFI_SUCCESS,0,NULL);
}

static VOID setup_acpi(boot_info_t *boot_info){
	EFI_CONFIGURATION_TABLE *conf_tb = ST->ConfigurationTable;

	EFI_GUID acpi1_guid = ACPI_TABLE_GUID;
	EFI_GUID acpi2_guid = ACPI_20_TABLE_GUID;
	for (UINTN i = 0;i < ST->NumberOfTableEntries;i++){
		if (CompareGuid(&conf_tb[i].VendorGuid,&acpi1_guid) == 0 || \
CompareGuid(&conf_tb[i].VendorGuid,&acpi2_guid) == 0){
			boot_info->acpi_rdsp = CONV_PTR(conf_tb[i].VendorTable);
			break;
		}
	}
	if (!boot_info->acpi_rdsp) Fatality(LDR_ACPI, 0);
}

static VOID setup_video(boot_info_t *boot_info, INTN tw, INTN th){
	Print(L"tw=%lld th=%lld\n", tw, th);

	//get videomode
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
	EFI_STATUS status = LibLocateProtocol(&gEfiGraphicsOutputProtocolGuid, (VOID**)&gop);
	if (EFI_ERROR(status)){
		Fatality(LDR_GOP, 0);
	}
	//Print(L"D: GOP found successfully\n");

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo, nativeMode;

	status = uefi_call_wrapper(gop->QueryMode, 4, gop, (gop->Mode==NULL)?0:gop->Mode->Mode, &SizeOfInfo, &info);
	nativeMode = gop->Mode->Mode;
	if(EFI_ERROR(status)) {
		Fatality(LDR_DISPLAY, 0);
	} else {
		status = uefi_call_wrapper(gop->SetMode, 2, gop, nativeMode);
	}

	UINTN numModes = gop->Mode->MaxMode;
	Print(L"nativeMode: %d, numModes: %d\n", nativeMode, numModes);
	for (UINT32 i=0; i < numModes; i++){
		status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);

		if (info->HorizontalResolution == tw && info->VerticalResolution == th){
			status = uefi_call_wrapper(gop->SetMode, 2, gop, i);
			goto final;
		}
	}

// there is no mode wanted in config
	#ifdef CUSTOM_VIDEOMODE
	status = uefi_call_wrapper(gop->SetMode, 2, gop, CUSTOM_VIDEOMODE);
	status = uefi_call_wrapper(gop->QueryMode, 4, gop, CUSTOM_VIDEOMODE, &SizeOfInfo, &info);
	#else
	status = uefi_call_wrapper(gop->QueryMode, 4, gop, nativeMode, &SizeOfInfo, &info);
	#endif

final:
	boot_info->vram = (VOID*)gop->Mode->FrameBufferBase;
	boot_info->vram_size = (UINT64)gop->Mode->FrameBufferSize;
	boot_info->width = (UINT16)info->HorizontalResolution;
	boot_info->height = (UINT16)info->VerticalResolution;
	boot_info->format = (UINT16)info->PixelFormat;
	boot_info->ppl = (UINT16)info->PixelsPerScanLine;
	
	Print(L"D: video memory at: %p size: %llu\n", boot_info->vram, boot_info->vram_size);
}
/*
VOID page_fault(UINT64 errcode, VOID* cr2){
	printf("Page fault during SetVirtualAddressMap! code %p addr %p\r\n", errcode, cr2);
	STOP();
}
*/
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
	InitializeLib(ImageHandle, SystemTable);
	uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

	register EFI_STATUS status; //one variable for all

	//set some f
	boot_info_t boot_info;

	setup_acpi(&boot_info);
	
	//load the kernel
	///open volume
	EFI_FILE_HANDLE root;
	status = OpenRoot(ImageHandle, &root);
	if (EFI_ERROR(status)) Fatality(LDR_DISK, status);

	CHAR8 *tsv;
	UINTN tsv_size = ReadFile(root, L"\\JUSTOS\\justboot.tsv", (VOID**)&tsv, 0);

	setup_video(
		&boot_info,
		tsv_parseint(tsv_search(tsv, "video", 1)),
		tsv_parseint(tsv_search(tsv, "video", 2))
	);

	CHAR16* kernelpath = tsv_parsestr(tsv_search(tsv, "kernel", 1));
	if (!kernelpath) Fatality(LDR_CFG, 0);
	Print(L"%s \n", kernelpath);
	FREE_KPAGES(tsv, tsv_size>>12);

	VOID *kernel;
	UINT64 k_filesz = ReadFile(root, kernelpath, &kernel, TO_PAGES(STACK_SIZE));
	FreePool(kernelpath);

	Print(L"D: kernel loaded at: %p\n", kernel);
	
	//check elf
	register UINT8 elf_status = check_elf(kernel);
	if (elf_status != 0) Fatality(LDR_ELF, elf_status);
	UINT64 entry = ((elf_hdr_t*)kernel)->e_entry;
	
	//EXIT BOOT SERVICES
	//uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

	EFI_MEMORY_DESCRIPTOR *mmap;
	UINTN mmapsize=1, mapkey, descsize, num_entries;
	UINT32 descver;

	GetMMap(&mmap, &mmapsize, &mapkey, &descsize, &descver, &num_entries);
	status = allocate_ptzone(&boot_info, mmap, num_entries, descsize);

	#ifdef DEBUG
	GetMMap(&mmap, &mmapsize, &mapkey, &descsize, &descver, &num_entries);
	register UINTN i = 0;
	for (;i < num_entries; i++) {
		register EFI_MEMORY_DESCRIPTOR *desc = MMAP(i);
		Print(L"%s %p %llu ", memtypeconvert(desc->Type), desc->PhysicalStart, desc->NumberOfPages);
		if (desc->Attribute&EFI_MEMORY_UC) Print(L"UC ");
		if (desc->Attribute&EFI_MEMORY_WC) Print(L"WC ");
		if (desc->Attribute&EFI_MEMORY_WT) Print(L"WT ");
		if (desc->Attribute&EFI_MEMORY_WB) Print(L"WB ");
		if (desc->Attribute&EFI_MEMORY_UCE) Print(L"UCE ");
		if (desc->Attribute&EFI_MEMORY_RP) Print(L"WP ");
		if (desc->Attribute&EFI_MEMORY_WP) Print(L"RP ");
		if (desc->Attribute&EFI_MEMORY_XP) Print(L"XP ");
		if (desc->Attribute&EFI_MEMORY_RUNTIME) Print(L"RT ");
		Print(L"\n");
	}
	#endif

	register UINT8 attempts = 0;
	asm volatile ("cli \n":::"memory");

	do {
		attempts++;
		Print(L"ExitBootServices - attempt %hu... ", attempts);
		GetMMap(&mmap, &mmapsize, &mapkey, &descsize, &descver, &num_entries);
		status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, mapkey);
		if (!EFI_ERROR(status)) break;
		Print(L"failed\n");
	} while (attempts < 4);
	if (attempts == 4){
		Print(L"Mission failed! Retreat! See you next time, kernel...\n");
		STOP();
	}	

	init_video(boot_info.vram, boot_info.width, boot_info.height);

	set_color(0x00ffffff, 0x00000000);
	//fill_rect(0x00ffffff, 50, 50, 50, 50);
	//draw_pixel(0x0000ff00, 1919, 1079);
	for (register UINT8 i = 0; i < 25; i++) printf("\r\n");
	printf("ExitBootServices() - success!\r\n");
	printf("descriptor size:%u\r\n", (UINT64)descsize);
	printf("descriptor ver:%u\r\n", (UINT64)descver);

	printf("0123456789abcdef\r\nghijklmnopqrstuvwxyz\r\n!?#$%%&\"'()*+-,./@\\\r\nABCDEFGHIJKLM\r\nNOPQRSTUVWXYZ[]{}:;`~_^|<=>\r\n");

	setup_paging(&boot_info, mmap, num_entries, descsize);
/*
	itdr.limit = 4095;
	itdr.base = boot_info.ptzone + boot_info.ptzone_size*4096 + 8192;
	asm volatile ("lidt %0" : : "m"(itdr));
	printf("idt changed\r\n");
*/
	//test_mdesc_order(mmap, descsize);

	#ifdef SETVAMAP
	status = uefi_call_wrapper(SystemTable->RuntimeServices->SetVirtualAddressMap, 4, mmapsize, descsize, descver, mmap);
	//set_color(0x00ffffff, 0x00000000);
	printf("SetVirtualAddressMap() - %u!\r\n", (UINT64)status);
	if (EFI_ERROR(status)) STOP();
	boot_info.rtsvcs = CONV_PTR(SystemTable->RuntimeServices);
	#else
	boot_info.rtsvcs = NULL;
	#endif

	convert_mmap_to_e820(&boot_info, mmap, num_entries, descsize);

	//start kernel
	if (entry < 0x1000) entry += 0x1000;
	register UINT64 stack = (UINT64)CONV_PTR(kernel+k_filesz);

	stack &= UINT64_MAX^0xF;

	printf("boot_info: %p\r\n", boot_info);
	printf("kernel: %p\r\n", CONV_PTR(kernel));
	printf("e_entry: %p\r\n", entry);
	printf("stack: %p\r\n", stack);
	printf("ptzone: %p\r\n", boot_info.ptzone);
	
	//convert boot_info pointers
	boot_info.mmap = CONV_PTR(boot_info.mmap);
	boot_info.vram = CONV_PTR(boot_info.vram);
	boot_info.ptzone = CONV_PTR(boot_info.ptzone);

	//copy boot_info to kernel
	RtCopyMem((UINT8*)kernel+4096, &boot_info, sizeof(boot_info));

	printf("We are ready to start!\r\n");
	#ifdef DEBUG
	STOP()
	#endif

	asm volatile ("":::"memory");

	start_kernel(CONV_PTR((UINT8*)kernel+4096), CONV_PTR(entry+kernel), \
	(VOID*)stack, (VOID*)((UINT64)boot_info.ptzone&MAX_PHY_ADDR));
	
	//HOW??
	return EFI_SUCCESS;
}
