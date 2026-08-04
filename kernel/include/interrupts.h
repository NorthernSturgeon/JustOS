#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

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

#define disable_irq() asm volatile ("cli \n\t" ::: "memory")
#define enable_irq() asm volatile ("sti \n\t" ::: "memory")

extern idtr_t idtr;
extern void fill_idt(IntDesc64 *ent, uint64_t ptr, uint16_t ss, uint8_t ist, uint8_t type);
extern void isr_start();

#endif