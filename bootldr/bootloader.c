#include <efi/efi.h>
#include <efi/efilib.h>

#include "serial.h"
#include "elf.h"
#include "ldrerr.h"
#include "lib/video.h"
#include "lib/console.h"

//#define ErrorCheck(actual, expected, msg) if(actual != expected) {Print(msg);return actual;}

//#define CONV_PTR(p) ((VOID*)(((UINT64)(p))|(0xffff8ull<<44)))
//                                          6456484032241608
#define CONV_PTR(p) (void*)(((UINT64)(p))|0xffff800000000000ull)

#define STOP for(;;)

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
	VOID *kernel;
	VOID *rtsvcs;
	VOID *acpi_rdsp;
	page_entry_t *ptzone;
	UINT64 ptzone_size;
	EFI_MEMORY_DESCRIPTOR *desc_list;
	UINT64 desc_size;
	VOID *vram;
	UINT64 vram_size;
	UINT16 width;
	UINT16 height;
	UINT16 format;
	UINT16 ppl;
} boot_info_t;

//#define BS_PTZONE_ALLOC
#define ovmf_r11337
//#define NATIVE_VIDEOMODE

#define STACK_SIZE 32768
//                     6347393123150700
#define MAX_PHY_ADDR 0x00007fffffffffffull

#ifdef ovmf_r11337
#define KERNEL_MEMORY_TYPE EfiRuntimeServicesData
#else
#define KERNEL_MEMORY_TYPE 0x80000000
#endif

#define ALLOC_KPOOL(ptr, size) uefi_call_wrapper(BS->AllocatePool, 3, KERNEL_MEMORY_TYPE, size, ptr)
#define FREE_KPOOL(ptr) uefi_call_wrapper(BS->FreePool, 1, ptr)
#define ALLOC_KPAGES(ptr, size) uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, KERNEL_MEMORY_TYPE, size, ptr)
#define FREE_KPAGES(ptr, size) uefi_call_wrapper(BS->FreePages, 2, ptr, size)

#define MMAP(i) ((EFI_MEMORY_DESCRIPTOR*)((UINT8*)mmap+descsize*i))

#define VALID_DESC(desc) ((desc->Type<15 || desc->Type==KERNEL_MEMORY_TYPE) && desc->NumberOfPages && !(desc->PhysicalStart&4095))
#define PTZONEMEM(desc) ((desc->Type==EfiBootServicesData)||(desc->Type==EfiBootServicesCode)||(desc->Type==EfiConventionalMemory))

//                     6448403224160800
#define PTENTRY_MASK 0x0000fffffffff000ull

#define GET_PTE(addr) ((page_entry_t*)((UINT64)(addr)&PTENTRY_MASK))
#define SET_PTE(addr) ((((UINT64)(addr))&PTENTRY_MASK)|3)

extern VOID start_kernel(boot_info_t *boot_info, VOID *k_entry, VOID *k_stack, VOID *cr3);

/*
static const char* memtypeconvert(EFI_MEMORY_TYPE memtype){
	switch (memtype){
	case EfiReservedMemoryType:
		return "reserved";
	case EfiLoaderCode:
		return "ldr-code";
	case EfiLoaderData:
		return "ldr-data";
	case EfiBootServicesCode:
		return "BS-code ";
	case EfiBootServicesData:
		return "BS-data ";
	case EfiRuntimeServicesCode:
		return "RT-code ";
	case EfiRuntimeServicesData:
		return "RT-code ";
	case EfiConventionalMemory:
		return "conv-mem";
	case EfiUnusableMemory:
		return "unusable";
	case EfiACPIReclaimMemory:
		return "ACPIrecl";
	case EfiACPIMemoryNVS:
		return "ACPI-NVS";
	case EfiMemoryMappedIO:
		return "MMIO    ";
	case EfiMemoryMappedIOPortSpace:
		return "MMIO-PS ";
	case EfiPalCode:
		return "palcode ";
	default:
		return "unknown ";
	}
}*/

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
	ALLOC_KPOOL(mmap, (*mmapsize)+2*(*descsize));
	status = uefi_call_wrapper(BS->GetMemoryMap, 5, mmapsize, *mmap, mapkey, descsize, descver);
	while (status!=EFI_SUCCESS){
		//Print(L"Attempt %u: Size=%u\n", i, *mmapsize);
		FREE_KPOOL(mmap);
		*mmapsize += 2*(*descsize);
		ALLOC_KPOOL(mmap, *mmapsize);
		status = uefi_call_wrapper(BS->GetMemoryMap, 5, mmapsize, *mmap, mapkey, descsize, descver);
	}
	*num_entries = *mmapsize / *descsize;
	//Print(L"Success! Size=%u\n", *mmapsize);
}

EFI_STATUS OpenRoot(EFI_HANDLE imgHand, EFI_FILE_HANDLE *file){
	EFI_LOADED_IMAGE_PROTOCOL* loadedImg;
	uefi_call_wrapper(BS->HandleProtocol, 3, imgHand, &gEfiLoadedImageProtocolGuid, (void**)&loadedImg);
	
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* filesystem;
	uefi_call_wrapper(BS->HandleProtocol, 3, loadedImg->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&filesystem);

	return uefi_call_wrapper(filesystem->OpenVolume, 2, filesystem, file);
}

EFI_STATUS LoadFile(EFI_FILE_HANDLE fileDir, EFI_FILE_HANDLE *file, CHAR16 *filename, EFI_FILE_INFO **fileinfo){
	register EFI_STATUS status = uefi_call_wrapper(fileDir->Open, 4, fileDir, file, filename, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (EFI_ERROR(status)) return status;

	*fileinfo = LibFileInfo(*file);

	return EFI_SUCCESS;
}
#ifdef BS_PTZONE_ALLOC
EFI_STATUS allocate_ptzone(boot_info_t *boot_info, EFI_MEMORY_DESCRIPTOR *mmap, UINTN num_entries, UINTN descsize){
	UINT64 totalsize=0;
	for(UINT64 cur_entry=0;cur_entry<num_entries;cur_entry++){
		EFI_MEMORY_DESCRIPTOR *desc = MMAP(cur_entry);
		if (VALID_DESC(desc)) totalsize += desc->NumberOfPages;
	}
	Print(L"Total memory size: %llu\n", totalsize<<12);
	boot_info->ptzone = (page_entry_t*)0xFFFFFFFFull;
	Print(L"PTZONE size: %llu\n", totalsize<<4);
	Print(L"PTZONE size: %llu\n", totalsize>>8);
	EFI_STATUS status = uefi_call_wrapper(BS->AllocatePages, 4,AllocateMaxAddress, EfiRuntimeServicesData, totalsize>>8, &boot_info->ptzone);
	if (status == EFI_OUT_OF_RESOURCES) {
		Print(L"Error: not enough memory!\n");
	} else if (status == EFI_INVALID_PARAMETER){
		Print(L"Error: parameter %p %p!\n", &boot_info->ptzone, boot_info->ptzone);
	} else if (EFI_ERROR(status)){
		Print(L"Error: unknown!\n");
	} else {
		Print(L"PTZONE: %p\n", boot_info->ptzone);
		Print(L"To clear in PTZONE: %llu\n", totalsize<<4);
		Print(L"Clear PTZONE... ");
		memset(boot_info->ptzone, 0, totalsize<<4);
		Print(L"success!\n");
		boot_info->ptzone_size = totalsize>>8;
	}
	return status;
}
#endif

VOID alloc_page(boot_info_t *boot_info, virt_addr_t vaddr, UINT64 *free_ptzonesz, UINT16 pml4offset){
	page_entry_t *cur_pt, *prev_pt;
	cur_pt = GET_PTE(boot_info->ptzone[vaddr.f.pml4+pml4offset].value); //get pdt from pml4
	if (cur_pt == NULL) { //pdt doesn't exists
		cur_pt = boot_info->ptzone + (boot_info->ptzone_size - (*free_ptzonesz)--)*512; //create pdt
		//set_color(0x00ff0000, 0x00000000);
		//printf("PDT: %p %p\r\n", vaddr.addr, cur_pt);
		boot_info->ptzone[vaddr.f.pml4+pml4offset].value = SET_PTE(cur_pt); //set pdt in pml4
	}
	prev_pt = cur_pt; //save current pdt
	cur_pt = GET_PTE(cur_pt[vaddr.f.pdt].value); //get pd in pdt
	if (cur_pt == NULL) { //pd doesn't exists
		cur_pt = boot_info->ptzone + (boot_info->ptzone_size - (*free_ptzonesz)--)*512; //create pd
		//set_color(0x0000ff00, 0x00000000);
		//printf("PD : %p %p\r\n", vaddr.addr, cur_pt);
		prev_pt[vaddr.f.pdt].value = SET_PTE(cur_pt); //set pd in pdt
	}
	prev_pt = cur_pt; //save current pd
	cur_pt = GET_PTE(cur_pt[vaddr.f.pd].value); //get pt from pd
	if (cur_pt == NULL) { //pt doesn't exists
		cur_pt = boot_info->ptzone + (boot_info->ptzone_size - (*free_ptzonesz)--)*512; //create pt
		//set_color(0x00ffff00, 0x00000000);
		//printf("PT : %p %p\r\n", vaddr.addr, cur_pt);
		prev_pt[vaddr.f.pd].value = SET_PTE(cur_pt); //set pt in pd
	}
	cur_pt[vaddr.f.pt].value = SET_PTE(vaddr.addr); //set page in pt
	//if (!(vaddr.addr&(0xFFFFFu))) printf("%p -> %p\r\n", vaddr.addr, cur_pt);
}


VOID setup_paging(boot_info_t *boot_info, EFI_MEMORY_DESCRIPTOR *mmap, UINTN num_entries, UINTN descsize){
	UINT64 cur_entry;
	virt_addr_t vaddr;

	#ifndef BS_PTZONE_ALLOC
	page_entry_t *ptzone;
	UINT64 cursize=0;
	UINT64 totalsize=0;
	for(cur_entry=0;cur_entry<num_entries;cur_entry++){
		EFI_MEMORY_DESCRIPTOR *desc = MMAP(cur_entry);
		if (VALID_DESC(desc)) totalsize += desc->NumberOfPages;
		if (desc->Attribute&EFI_MEMORY_RUNTIME){
			desc->VirtualStart = (desc->PhysicalStart)|(0xffff8ull<<44);
		}
		if (PTZONEMEM(desc)){
			ptzone = (page_entry_t*)desc->PhysicalStart;
			cursize = desc->NumberOfPages;
		}
		if (cursize>boot_info->ptzone_size) {
			boot_info->ptzone_size = cursize;
			boot_info->ptzone = ptzone;
		}
	}
	printf("total memory: %u\r\n", totalsize);
	printf("PTZONE_SIZE: %u\r\n", boot_info->ptzone_size);
	printf("PTZONE: %p\r\n", boot_info->ptzone);
	printf("To clear in PTZONE: %u\r\n", totalsize<<4); // pages -> bytes (2^12) -> (1^-9)*2
	printf("Clear PTZONE... ");
	ZeroMem(boot_info->ptzone, totalsize<<4); //fixed 2025-07-15 v0.0.2
	printf("success!\r\n");
	boot_info->ptzone_size = totalsize>>8; // (1^-9)*2
	#endif
	UINT64 free_ptzonesz=boot_info->ptzone_size-1; //first page of boot_info->ptzone is pml4
	printf("free_ptzonesz (bytes): %u\r\n", free_ptzonesz<<12);
	printf("start kernel at: %p\r\n", start_kernel); //cr3 reload in start_kernel, so it must be allocated in both higher and lower halves
	printf("1st startkernel page: %p\r\n", (virt_addr_t)((uint64_t)start_kernel&(MAX_ADDRESS<<12)));
	printf("2nd startkernel page: %p\r\n", (virt_addr_t)(((uint64_t)start_kernel+4096)&(MAX_ADDRESS<<12)));
	alloc_page(boot_info, (virt_addr_t)((uint64_t)start_kernel&(MAX_ADDRESS<<12)), &free_ptzonesz, 0);
	alloc_page(boot_info, (virt_addr_t)(((uint64_t)start_kernel+4096)&(MAX_ADDRESS<<12)), &free_ptzonesz, 0);
	for (vaddr.addr = (UINT64)boot_info->vram; 
		vaddr.addr < (UINT64)boot_info->vram + boot_info->vram_size;
		vaddr.addr += 4096) {
		alloc_page(boot_info, vaddr, &free_ptzonesz, 256);
	}

	for(cur_entry=0;cur_entry<num_entries;cur_entry++){
		EFI_MEMORY_DESCRIPTOR *desc = MMAP(cur_entry);
		if (!VALID_DESC(desc)) continue;
		for (vaddr.addr=desc->PhysicalStart; vaddr.addr<desc->NumberOfPages*4096+desc->PhysicalStart;vaddr.addr+=4096){
			if (free_ptzonesz <= 3) goto error;
			alloc_page(boot_info, vaddr, &free_ptzonesz, 256);
		}
	}

	boot_info->ptzone_size -= free_ptzonesz;
	printf("PTZONE true size: %u\r\n", boot_info->ptzone_size<<12);
	boot_info->ptzone = CONV_PTR(boot_info->ptzone);
	return;
	error:
	printf("Out of memory!\r\n");
	//uefi_call_wrapper(SystemTable->RuntimeServices,4,EfiResetShutdown,EFI_SUCCESS,0,NULL);
}

VOID setup_acpi(boot_info_t *boot_info){
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

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable){
	InitializeLib(ImageHandle, SystemTable);
	uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

	register EFI_STATUS status; //one variable for all

	//set some f
	boot_info_t *boot_info;
	status = ALLOC_KPOOL(sizeof(boot_info_t), &boot_info);

	setup_acpi(boot_info);

	//get videomode
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

	status = LibLocateProtocol(&gEfiGraphicsOutputProtocolGuid, (VOID**)&gop);
	if (EFI_ERROR(status)){
		Fatality(LDR_GOP, 0);
	}
	//Print(L"D: GOP found successfully\n");

	//Print(L"%p\n", ST->ConfigurationTable);

	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	UINTN SizeOfInfo, nativeMode, numModes;

	//status = uefi_call_wrapper(gop->SetMode, 2, gop, 0);
	status = uefi_call_wrapper(gop->QueryMode, 4, gop, (gop->Mode==NULL)?0:gop->Mode->Mode, &SizeOfInfo, &info);
	if(EFI_ERROR(status)) {
		Fatality(LDR_DISPLAY, 0);
	} else {
		nativeMode = gop->Mode->Mode;
		status = uefi_call_wrapper(gop->SetMode, 2, gop, nativeMode);
		numModes = gop->Mode->MaxMode;
	}

	Print(L"nativeMode: %d, numModes: %d\n", nativeMode, numModes);

	for (UINT32 i=0; i < numModes; i++){
		status = uefi_call_wrapper(gop->QueryMode, 4, gop, i, &SizeOfInfo, &info);
		
		Print(L"D: mode %03d width %d height %d ppl %d format %d\n", i, \
			info->HorizontalResolution, info->VerticalResolution, \
			info->PixelsPerScanLine, info->PixelFormat);
	}

	#ifndef NATIVE_VIDEOMODE
	status = uefi_call_wrapper(gop->SetMode, 2, gop, 12);
	status = uefi_call_wrapper(gop->QueryMode, 4, gop, 12, &SizeOfInfo, &info);
	#else
	status = uefi_call_wrapper(gop->QueryMode, 4, gop, nativeMode, &SizeOfInfo, &info);
	#endif
	boot_info->vram = (VOID*)gop->Mode->FrameBufferBase;
	boot_info->vram_size = (UINT64)gop->Mode->FrameBufferSize;
	boot_info->width = (UINT16)info->HorizontalResolution;
	boot_info->height = (UINT16)info->VerticalResolution;
	boot_info->format = (UINT16)info->PixelFormat;
	boot_info->ppl = (UINT16)info->PixelsPerScanLine;
	
	Print(L"D: video memory at: %p size: %llu\n", boot_info->vram, boot_info->vram_size);
	
	//load the kernel
	///open volume
	EFI_FILE_HANDLE root;
	status = OpenRoot(ImageHandle, &root);
	if (EFI_ERROR(status)) Fatality(LDR_DISK, status);
	EFI_FILE_HANDLE k_file;

	EFI_FILE_INFO *k_fileinfo;
	status = LoadFile(root, &k_file, L"\\EFI\\Boot\\kernel.elf", &k_fileinfo);
	if (EFI_ERROR(status)) Fatality(LDR_FILE, 0);

	Print(L"file size: %llu\n", k_fileinfo->FileSize);
	UINT64 k_filesz = k_fileinfo->FileSize + STACK_SIZE;

	//Print(L"filesz: %llu\n", k_filesz);
	///read kernel
	VOID *kernel; // (k_filesz+4096-k_filesz%4096)/4096 or k_filesz/4096+(k_filesz%4096)?1:0 ???
	status = ALLOC_KPAGES(&kernel, (k_filesz>>12)+((k_filesz&4095)?1:0));
	status = uefi_call_wrapper(k_file->Read, 3, k_file, &k_filesz, kernel);
	FreePool(k_fileinfo);
	if (EFI_ERROR(status)) Fatality(LDR_FILE, status);

	Print(L"D: kernel loaded at: %p\n", kernel);
	
	///close kernel's file
	uefi_call_wrapper(k_file->Close, 1, k_file);
	
	//check elf
	register UINT8 elf_status = check_elf(kernel);
	if (elf_status != 0) Fatality(LDR_ELF, elf_status);

	boot_info->kernel = CONV_PTR(kernel);
	
	//EXIT BOOT SERVICES
	//uefi_call_wrapper(ST->ConOut->ClearScreen, 1, ST->ConOut);

	EFI_MEMORY_DESCRIPTOR *mmap;
	UINTN mmapsize=1, mapkey, descsize, num_entries;
	UINT32 descver;
	#ifdef BS_PTZONE_ALLOC
	GetMMap(&mmap, &mmapsize, &mapkey, &descsize, &descver, &num_entries);
	status = allocate_ptzone(boot_info, mmap, num_entries, descsize);
	#endif
	
	//UINT8 attempts;
	GetMMap(&mmap, &mmapsize, &mapkey, &descsize, &descver, &num_entries);
	asm volatile ("cli \n":::"memory");
	status = uefi_call_wrapper(BS->ExitBootServices, 2, ImageHandle, mapkey);

	init_video(boot_info->vram, boot_info->width, boot_info->height);
	set_color(0x00ffffff, 0x00000000);
	fill_rect(0x0000ffff, 50, 50, 50, 50);
	//draw_pixel(0x0000ff00, 1919, 1079);
	printf("ExitBootServices() - success!\r\n");
	printf("descriptor size:%u\r\n", descsize);
	printf("descriptor ver:%u\r\n", descver);
	/*
	register UINTN i = 0;
	for (;i < num_entries; i++) {
		register EFI_MEMORY_DESCRIPTOR *desc = (EFI_MEMORY_DESCRIPTOR*)((UINT8*)mmap+descsize*i);
		if (desc->Type < EfiMaxMemoryType && desc->NumberOfPages && !(desc->PhysicalStart&4095)){
			printf("%s %p %u\r\n", memtypeconvert(desc->Type), desc->PhysicalStart, desc->NumberOfPages);
		}
	}*/

	printf("0123456789abcdef\r\nghijklmnopqrstuvwxyz\r\n!?#$%%&\"'()*+-,./@\\\r\nABCDEFGHIJKLM\r\nNOPQRSTUVWXYZ[]{}:;`~_^|<=>\r\n");

	boot_info->desc_list = CONV_PTR(mmap);
	boot_info->desc_size = (UINT64)mmapsize;
	setup_paging(boot_info, mmap, num_entries, descsize);

	status = uefi_call_wrapper(RT->SetVirtualAddressMap, 4, mmapsize, descsize, descver, mmap);
	boot_info->rtsvcs = SystemTable->RuntimeServices;
	boot_info->ptzone = CONV_PTR(boot_info->ptzone);
	//set_color(0x00ffffff, 0x00000000);
	printf("SetVirtualAddressMap() - %u!\r\n", status);

	//start kernel
	if (((elf_hdr_t*)kernel)->e_entry < 0x1000) ((elf_hdr_t*)kernel)->e_entry += 0x1000;
	printf("boot_info: %p\r\n", boot_info);
	printf("kernel: %p\r\n", CONV_PTR(kernel));
	printf("e_entry: %p\r\n", ((elf_hdr_t*)kernel)->e_entry);
	printf("stack: %p\r\n", CONV_PTR(kernel+k_filesz));
	printf("ptzone: %p\r\n", boot_info->ptzone);
	//for(;;);
	
	boot_info->vram = CONV_PTR(boot_info->vram);
	start_kernel(CONV_PTR(boot_info), CONV_PTR(((elf_hdr_t*)kernel)->e_entry+kernel), \
	CONV_PTR(kernel+k_filesz), boot_info->ptzone);
	
	//HOW??
	return EFI_SUCCESS;
}
