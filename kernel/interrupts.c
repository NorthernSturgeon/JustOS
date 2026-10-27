#include "interrupts.h"
#include "lib/console.h"
#include "register.h"

idtr_t idtr __attribute__((aligned(16)));

static uint16_t repeat = 0;

//                            #DF           #TS #NP #SS #GP #PF            #AC             #CP             #VC             #SX
#define HAS_CODE(intv) ((intv == 8) || (intv >= 10 && intv <= 14) || (intv == 17) || (intv == 21) || (intv == 29) || (intv == 30))

bool isr_common(uint8_t intn, uint64_t* stack){
	printf("%u %p %p %p %p %p %p\n", (uint64_t)intn, *(stack), *(stack+1), *(stack+2), *(stack+3), *(stack+4), *(stack+5));
	if (intn == 14){
		uint64_t cr2;
		read_rega(cr2, cr2);
		printf("PAGE FAULT CR2=%p ERRCODE=%p\n", cr2, *(stack));
		for (;;);
	} else if (intn == repeat || intn == 8){
		for (;;);
	}
	else if (intn < 32){
		repeat = intn;
		if (HAS_CODE(intn))	return 1;
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