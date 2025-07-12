#ifndef __BYTEORDER_H__
#define __BYTEORDER_H__

extern uint64_t chbo64(uint64_t value);
extern uint32_t chbo32(uint32_t value);
extern uint16_t chbo16(uint16_t value);

#define _chbo64(val) asm volatile ("rex.w bswap %0"::);
#define _chbo32(val) asm volatile ("bswap %0\n"::);
#define _chbo16(val) asm volatile ("":"m"(val):);

#endif