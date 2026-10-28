#include "lib/console.h"
#include "lib/font.h"
#include "lib/memory.h"
#include "boot.h"
#include "video.h"
#include "register.h"
#include "tls.h"
#include "interrupts.h"

extern void asm_hlt(void);

__tls uint64_t test_thread_data = 0x0102030405060708ull;

struct __packed BmpHeader{
	uint16_t magic;
	uint32_t bfsize;
	uint32_t bfreserved;
	uint32_t bfoffbits;
	uint32_t bisize;
	uint32_t biwidth;
	uint32_t biheight;
	uint16_t biplanes;
	uint16_t bibitcount;
	uint32_t bicompr;
	uint32_t bisizeimage;
	int32_t bixppm;
	int32_t biyppm;
	uint32_t biclrused;
	uint32_t biclrimporntant;
};

void render_logo(struct BmpHeader *logo_hdr){
	if (logo_hdr->bibitcount != 24 || boot_info->width < logo_hdr->biwidth ||
		boot_info->height < logo_hdr->biheight || logo_hdr->bisize != 40 ||
		logo_hdr->bicompr != 0
	) return;

	uint32_t xc = boot_info->width / 2 - logo_hdr->biwidth / 2;
	uint32_t yc = boot_info->height / 2 - logo_hdr->biheight / 2;

	uint32_t scanline = round_to((logo_hdr->bibitcount/8)*logo_hdr->biwidth, 4);
	uint8_t *bitmap = (uint8_t*)logo_hdr + logo_hdr->bfoffbits;

	for (uint32_t cy = 0; cy < logo_hdr->biheight; cy++){
		for (uint32_t cx = 0; cx < logo_hdr->biwidth; cx++){
			uint32_t color = (*(uint32_t*)(&bitmap[cy*scanline + cx*3])) >> 8;

			color = ((color & 0x00FF0000u)) | ((color & 0x000000FFu) << 8) | ((color & 0x0000FF00u) >> 8);

			draw_pixel(color, xc + cx, yc + (logo_hdr->biheight - cy));
		}
	}
	
	printf("Rendered!\n");
}

void arena_test2(){
	set_column_width(70);
	void **ptrs = malloc(200*sizeof(void*));
	for (size_t iter = 0; iter < 4; iter++) {
		set_color(rgb(iter*80, 255, 255), COLOR_BLACK);
		for (size_t i = 0; i < 200; i++){ 
			printf("%u ", i);
			ptrs[i] = malloc((i * 371 + 13) % 65534);
		}

		for (size_t i = 0; i < 200; i++){
			// printf("-%u ", i);
			free(ptrs[i]);
		}
		printf("..................................\n");
	}
}

void arena_test_destroy() {
	void **ptrs = malloc(505 * sizeof(void*));
	for (size_t i = 0; i < 505; i++) 
		ptrs[i] = malloc(9600);
	for (size_t i = 0; i < 505; i++) 
		free(ptrs[i]);
	void *trigger = malloc(65530);
	free(trigger);
}

void arena_test(){
	void* list[30] = {0};

	printf("Malloc test\n");
	size_t *data = malloc(32);

	printf("%p %u\n", data, *(data - 1));
	free(data);

	set_color(rgb(255, 128, 128), COLOR_BLACK);
	data = malloc(64000);
	free(data);

	set_color(rgb(225, 255, 0), COLOR_BLACK);
	for (size_t i = 0; i < 30; i++){
		list[i] = malloc(64000);
	}

	set_color(rgb(128, 255, 141), COLOR_BLACK);
	for (size_t i = 0; i < 20; i++) free(list[i]);

	set_color(rgb(127, 198, 255), COLOR_BLACK);
	for (size_t i = 0; i < 20; i++){
		list[i] = malloc(64000);
	}
}

void kmain(){
	init_video(boot_info->vram, boot_info->width, boot_info->height, boot_info->ppl, boot_info->format);
	fill_rect(0, 0, 0, boot_info->width-1, boot_info->height-1);

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
			printf("%u p", (gorl.list[i+1] - gorl.list[i])>>12);
		}

		printf("\n");
	}
	set_color(COLOR_WHITE, COLOR_BLACK);

	idtr.base = allocate_pages(1);
	idtr.limit = 4095;
	for (uint16_t i = 0; i < 256; i++){
		fill_idt(&idtr.base[i], (uint64_t)isr_start + i*7, 0x8, 0, INT_TYPE_INTERRUPT);
	}

	asm volatile ("lidt %0 \n\t" :: "m"(idtr) : "memory");
	printf("IDT loaded! \n");
	enable_irq();
	printf("Interrupts! \n");
	asm volatile ("int $255 \n\t" ::: "memory");

	uint64_t gsbase;
	rdmsr(IA32_GS_BASE, gsbase);
	printf("GS_BASE: %p\n", gsbase);
	printf("TLS TEST: %p\n", test_thread_data);

	printf("Splash %p / %u b\n", boot_info->splash, boot_info->splash_size);
	struct BmpHeader *logo = boot_info->splash;
	printf("offset %u width %u height %u bits %u\n", (uint64_t)logo->bfoffbits, (uint64_t)logo->biwidth, (uint64_t)logo->biheight, (uint64_t)logo->bibitcount);
	render_logo(logo);

	// --- FAST HEAP TEST ---
	clear_console();
	arena_test_destroy();

	//for(;;);
	asm_hlt();
	//boot_info->rtsvcs->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}