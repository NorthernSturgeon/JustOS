#include "lib/console.h"
#include "lib/memory.h"
#include "rtsvcs.h"
#include "boot.h"
#include "lib/font.h"
#include "video.h"
//#include "register.h"

extern void asm_hlt(void);
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

uint64_t test_global_data = BAD_POINTER;

void kmain(){
	init_video(boot_info->vram, boot_info->width, boot_info->height, boot_info->ppl, boot_info->format);
	fill_rect(0, 0, 0, boot_info->width-1, boot_info->height-1);
	/*
	itdr.limit = 4095;
	itdr.base = (void*)boot_info - 4096;
	asm volatile ("lidt %0" : : "m"(itdr));
	*/
	set_color(COLOR_WHITE, COLOR_BLACK);

	//cr0_t cr0;
	//read_reg(cr0.value, cr0);

	printf("Hello from dynamic kernel!\n");
	printf("BTinfo: %p\n", boot_info);
	printf("RTsvcs: %p\n", boot_info->rtsvcs);
	printf("ACPIrp: %p\n", boot_info->acpi_rdsp);
	printf("STACK : %p\n", boot_info->stack);
	printf("STCKsz: %u\n", boot_info->stack_size);
	printf("KRNLsz: %u\n", boot_info->kernel_size);
	printf("PTZONE: %p\n", boot_info->ptzone);
	printf("PTZ_sz: %u\n", boot_info->ptzone_size);
	printf("MMAP  : %p\n", boot_info->mmap);
	printf("MMAPsz: %u\n", boot_info->mmap_len);
	printf("VRAM  : %p\n", boot_info->vram);
	printf("VRAMsz: %u\n", boot_info->vram_size);
	printf("videoformat: %ux%u %u\n", (uint64_t)boot_info->width, (uint64_t)boot_info->height, (uint64_t)boot_info->format);
/*
	set_color(COLOR_GRAY, COLOR_BLACK);
	printf("base             | lenght           | type | attr\n");
	for(size_t i=0; i < boot_info->mmap_len; i++){
		printf("%p | %p | %u    | %u\n" , boot_info->mmap[i].base, boot_info->mmap[i].lenght, boot_info->mmap[i].type, boot_info->mmap[i].attr);
	}

	set_color(COLOR_WHITE, COLOR_BLACK);
*/
	printf("Initializing memory manager...\n");
	init_mm();

	// set_color(COLOR_GRAY, COLOR_BLACK);
	// printf("base             | lenght           | type | attr\n");
	// for(size_t i=0; i < e820_len; i++){
	// 	printf("%p | %p | %u    | %u\n" , e820[i].base, e820[i].lenght, e820[i].type, e820[i].attr);
	// }

	set_color(COLOR_WHITE, COLOR_BLACK);
	printf("gorl: \n");
	for (size_t i = 0; i < gorl.lenght; i++){
		set_color(0x00ff7f7f, 0);
		printf("%p ", gorl.list[i++]);

		set_color(COLOR_WHITE, 0);
		printf("%u p - ", (gorl.list[i] - gorl.list[i-1])>>12);

		//set_color(COLOR_WHITE, 0);
		//printf(" - ");

		set_color(0x007f7fff, 0);
		printf("%p ", gorl.list[i]);

		if (i < gorl.lenght - 1) {
			set_color(COLOR_WHITE, 0);
			printf("%u p\n", (gorl.list[i+1] - gorl.list[i])>>12);
		}
	}
	set_color(COLOR_WHITE, COLOR_BLACK);
	//for (size_t i = 0; i < 200; i++) printf("Line speedtest: %u\n", i);

	//for(;;);
	asm_hlt();
	//boot_info->rtsvcs->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}