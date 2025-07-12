#include "lib/types.h"
#include "lib/console.h"
#include "rtsvcs.h"
#include "boot.h"
#include "video.h"

extern void _asm_ijmp(void);

void kmain(boot_info_t *boot_info){
	//boot_info->rtsvcs->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
	init_video(boot_info->vram, boot_info->width, boot_info->height);
	//set_color(0x00000000, 0x00ffff00);
	*((uint32_t*)(boot_info->vram+1)) = 0x00ffff00;
	fill_rect(0x00ffff00, 50, 50, 50, 50);
	//printf("abcdef");
	for(;;);
	_asm_ijmp();
	boot_info->rtsvcs->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
}