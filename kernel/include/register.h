#ifndef __REGS_H__
#define __REGS_H__

typedef union{
	uint32_t value;
	struct {
		uint32_t pe:1;
		uint32_t mp:1;
		uint32_t em:1;
		uint32_t ts:1;
		uint32_t et:1;
		uint32_t ne:1;
		uint32_t resv0:10;
		uint32_t wp:1;
		uint32_t resv1:1;
		uint32_t am:1;
		uint32_t resv2:10;
		uint32_t nw:1;
		uint32_t cd:1;
		uint32_t pg:1;
		//uint64_t reserved3:32;
	} flags;
} cr0_t;

typedef union{
	uint64_t value;
	struct {
		uint64_t resv0:2;
		uint64_t pwt:1;
		uint64_t pcd:1;
		uint64_t resv1:8;
		uint64_t pml4_addr:36;
		uint64_t resv2:16;
	} flags;
} cr3_t;

typedef union{
	uint64_t value;
	struct {
		uint64_t vme:1;
		uint64_t pvi:1;
		uint64_t tsd:1;
		uint64_t de:1;
		uint64_t pse:1;
		uint64_t pae:1;
		uint64_t mce:1;
		uint64_t pge:1;
		uint64_t pce:1;
		uint64_t osfxsr:1;
		uint64_t osxmmexcpt:1;
		uint64_t umip:1;
		uint64_t la57:1;
		uint64_t vmxe:1;
		uint64_t smxe:1;
		uint64_t resv0:1;
		uint64_t fsgsbase:1;
		uint64_t pcide:1;
		uint64_t osxsave:1;
		uint64_t kle:1;
		uint64_t smep:1;
		uint64_t smap:1;
		uint64_t pke:1;
		uint64_t cet:1;
		uint64_t pks:1;
		uint64_t resv1:38;
	} flags;
} cr4_t;

#define read_reg(dest, reg) asm volatile ("mov %##reg, %0":"m"(dest):)
#define write_reg(reg, src) asm volatile ("mov %0, %##reg"::"m"(src))
#define write_value_reg(reg, value) asm volatile ("mov $value, %##reg"::)

/* ******************** MSRs ******************** */ 
                 
#define IA32_EFER 0xC0000080

#define IA32_STAR 0xC0000081
#define IA32_LSTAR 0xC0000082
#define IA32_CSTAR 0xC0000083
#define IA32_SFMASK 0xC0000084

typedef union{
	uint64_t value;
	struct {
		uint64_t sce:1;
		uint64_t resv0:7;
		uint64_t lme:1;
		uint64_t resv1:1;
		uint64_t lma:1;
		uint64_t nxe:1;
		uint64_t svme:1;
		uint64_t lmsle:1;
		uint64_t ffxsr:1;
		uint64_t tce:1;
		uint64_t resv2:48;
	} flags;
} ia32_efer_t;

extern uint64_t _rdmsr(uint32_t msr);
extern void _wrmsr(uint32_t msr, uint64_t value);

#endif