#ifndef __STRING_H__
#define __STRING_H__

extern void memset(void* dest, char c, size_t n);
extern void memcpy(void* dest, void* src, size_t n);
//extern void memccpy(void* dest, void* src, char c);
//extern void memnccpy(void* dest, void* src, char c, size_t n);
//extern size_t memcmp(void* src1, void* src2);

#define zeromem(dest, n) memset(dest, 0, n)
//#define strcpy(dest, src) memccpy((void*)dest, (void*)src, 0)

#endif