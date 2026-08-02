#include "lib/console.h"
#include "lib/font.h"
#include "physmem.h"
#include "boot.h"
#include "video.h"
#include "register.h"
#include "tls.h"

extern void asm_hlt(void);

#define INT_TYPE_INTERRUPT 0x8e
#define INT_TYPE_TRAP 0x8f

typedef struct __packed {
	uint16_t offset1;        // offset bits 0..15
	uint16_t selector;        // a code segment selector in GDT or LDT
	uint8_t  ist;             // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
	uint8_t  type_attributes; // gate type, dpl, and p fields
	uint16_t offset2;        // offset bits 16..31
	uint32_t offset3;        // offset bits 32..63
	uint32_t zero;            // reserved
} IntDesc64;

typedef struct __packed {
	uint16_t limit;
	IntDesc64 *base;
} idtr_t;

idtr_t idtr __attribute__((aligned(16)));

uint64_t __tls test_thread_data = 0x0102030405060708ull;

extern void isr_start();

static uint16_t repeat = 0;

uint8_t isr_common(uint8_t intn, uint64_t* stack){
	printf("%u %p %p %p %p %p %p\n", (uint64_t)intn, *(stack), *(stack+1), *(stack+2), *(stack+3), *(stack+4), *(stack+5));
	if (intn == repeat){
		for (;;);
	}
	else if (intn < 32){
		repeat = intn;
		return 1;
	}
	return 0;
}

__export void fill_idt(IntDesc64 *ent, uint64_t ptr, uint16_t ss, uint8_t ist, uint8_t type){
	ent->offset1 = ptr & 0xFFFF;
	ent->offset2 = (ptr >> 16) & 0xFFFF;
	ent->offset3 = (uint32_t)(ptr >> 32);

	ent->selector = ss;
	ent->ist = ist;
	ent->type_attributes = type;
	ent->zero = 0;
}

void kmain(){
	init_video(boot_info->vram, boot_info->width, boot_info->height, boot_info->ppl, boot_info->format);
	fill_rect(0, 0, 0, boot_info->width-1, boot_info->height-1);
	/*
	idtr.limit = 4095;
	idtr.base = (void*)boot_info - 4096;
	asm volatile ("lidt %0" : : "m"(idtr));
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
/*
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
	//for (size_t i = 0; i < 200; i++) printf("Line speedtest: %u\n", i);

	uint64_t gsbase;
	rdmsr(IA32_GS_BASE, gsbase);
	printf("GS BASE: %p\n", gsbase);
	printf("TLS TEST: %p\n", *tls_ptr(test_thread_data));
	//printf("TLS TEST: %p\n", test_global_data);
*/
	idtr.base = allocate_pages(1);
	idtr.limit = 4095;
	for (uint16_t i = 0; i < 256; i++){
		fill_idt(&idtr.base[i], (uint64_t)isr_start + i*7, 0x8, 0, INT_TYPE_INTERRUPT);
	}

	asm volatile ("lidt %0 \n\t" :: "m"(idtr) : "memory");
	printf("IDT loaded! \n");

	asm volatile ("sti \n\t" ::: "memory");
	printf("Interrupts! \n");

	asm volatile ("int $255 \n\t" ::: "memory");

	//for(;;);
	asm_hlt();
	//boot_info->rtsvcs->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
}