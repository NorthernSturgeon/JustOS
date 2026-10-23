#ifndef __MEMORY_H__
#define __MEMORY_H__

extern void* malloc(size_t size);
extern void free(void* ptr);

extern uint64_t round_to(uint64_t n, uint64_t k);

#endif