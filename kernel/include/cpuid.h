#ifndef __CPUID_H__
#define __CPUID_H__

typedef struct {
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
} cpuid_t;

extern void _cpuid(cpuid_t *values);

#endif