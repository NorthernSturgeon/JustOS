#include "lib/types.h"
#include "lib/console.h"
#include "rtsvcs.h"
#include "boot.h"
#include "lib/font.h"
#include "video.h"

extern void _asm_ijmp(void);
/*
typedef struct {
	uint16_t limit;
	void    *base;
} __packed idtr_t;

idtr_t itdr;
*/
void kmain(boot_info_t *boot_info){
	init_video(boot_info->vram, boot_info->width, boot_info->height);
	init_kernel_font(boot_info->kernel);
	fill_rect(0, 0, 0, boot_info->width-1, boot_info->height-1);
	/*
	itdr.limit = 4095;
	itdr.base = boot_info->ptzone + boot_info->ptzone_size*4096 + 8192;
	asm volatile ("lidt %0" : : "m"(itdr));
	*/
	set_color(0x00ffffff, 0x00000000);
	printf("Hello from kernel!\r\n");
	printf("Kernel: %p\r\n", boot_info->kernel);
	printf("RTsvcs: %p\r\n", boot_info->rtsvcs);
	printf("ACPIrp: %p\r\n", boot_info->acpi_rdsp);
	printf("PTZONE: %p\r\n", boot_info->ptzone);
	printf("PTZ_sz: %u\r\n", boot_info->ptzone_size);
	printf("DESC_l: %p\r\n", boot_info->desc_list);
	printf("DESC_s: %u\r\n", boot_info->desc_size);
	printf("VRAM  : %p\r\n", boot_info->vram);
	printf("VRAMsz: %u\r\n", boot_info->vram_size);
	//for(;;);
	_asm_ijmp();
	boot_info->rtsvcs->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
}