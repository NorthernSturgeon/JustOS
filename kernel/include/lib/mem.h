#ifndef __MEM_H__
#define __MEM_H__

extern void memcpy(void* dest, void* src, size_t size);
extern void memmove(void* dest, void* src, size_t size);
extern void memset(void* dest, uint8_t c, size_t size); 

//extern void* malloc(uint64_t size);
//extern void* free(void* ptr);

#endif