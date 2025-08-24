#ifndef __CPUID_H__
#define __CPUID_H__

typedef struct{
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
} cpuid_t;

inline void __cpuid(cpuid_t *values) {
	asm ("cpuid;":"+a"(values->eax), "=b"(values->ebx), "+c"(values->ecx), "=d"(values->edx):);
}

#endif