#ifndef __CPUID_H__
#define __CPUID_H__

typedef struct{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
} cpuid_t;

static __always_inline cpuid_t __cpuid(uint32_t eax, uint32_t ecx) {
	cpuid_t values = {
		.eax = eax,
		.ecx = ecx
	};

	asm ("cpuid;":"+a"(values.eax), "=b"(values.ebx), "+c"(values.ecx), "=d"(values.edx):);

	return values;
}

#endif