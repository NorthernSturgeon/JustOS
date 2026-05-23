#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

#define bswap64(val) asm ("bswap %0\n":"+r"(val));
#define bswap32(val) asm ("bswap %0\n":"+r"(val));
#define bswap16(val) asm ("xchg %%al, %%ah":"+a"(val));

#endif