#include "lib/types.h"
#include "lib/console.h"
#include "rtsvcs.h"
#include "boot.h"
#include "lib/font.h"
#include "video.h"
//#include "lib/memory.h"
//#include "register.h"

extern void _asm_ijmp(void);
/*
typedef struct {
	uint16_t limit;
	void    *base;
} __packed idtr_t;

idtr_t itdr;

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
		return "RT-data ";
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
}
*/

void kmain(){
	init_video(boot_info.vram, boot_info.width, boot_info.height);
	void *kernel = (void*)&boot_info;
	init_kernel_font(kernel-4096);
	fill_rect(0, 0, 0, boot_info.width-1, boot_info.height-1);
	/*
	itdr.limit = 4095;
	itdr.base = boot_info.ptzone + boot_info.ptzone_size*4096 + 8192;
	asm volatile ("lidt %0" : : "m"(itdr));
	*/
	set_color(0x00ffffff, 0x00000000);

	//cr0_t cr0;
	//read_reg(cr0.value, cr0);

	printf("Hello from kernel!\n");
	printf("BTinfo: %p\n", &boot_info);
	printf("RTsvcs: %p\n", boot_info.rtsvcs);
	printf("ACPIrp: %p\n", boot_info.acpi_rdsp);
	printf("PTZONE: %p\n", boot_info.ptzone);
	printf("PTZ_sz: %u\n", boot_info.ptzone_size);
	printf("BMAP  : %p\n", boot_info.bitmap);
	printf("BMAPsz: %u\n", boot_info.bitmap_size);
	printf("VRAM  : %p\n", boot_info.vram);
	printf("VRAMsz: %u\n", boot_info.vram_size);

	//init_mm(boot_info.desc_list, boot_info.desc_size/boot_info.size, boot_info.size);
	//printf("reg ft: %p\n", gmrtp.first_table);

	for(;;);
	//_asm_ijmp();
	//boot_info.rtsvcs->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}