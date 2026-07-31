#ifndef __TLS_H__
#define __TLS_H__

__export void* __attribute__((weak)) _DTV_;
extern void* _percpu_start_;

static inline void* tls_ref(size_t off){
	register void* ptr;
	asm (
		"movq _DTV_(%%rip), %0 \n\t"
		"movq %%gs:(%0), %0 \n\t"
		:"=r"(ptr)::
	);
	return ptr + off - (size_t)&_percpu_start_;
}

#define tls_ptr(var) ((__typeof__(var)*)tls_ref((size_t)&var))

#define __tls __attribute__((section(".percpu")))

#endif