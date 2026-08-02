#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stddef.h>

#define true 1
#define false 0

#define asm __asm__

#define __export __attribute__((visibility("default")))
#define __always_inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))
#define __packed __attribute__((__packed__))
#define __naked __attribute__((naked))

#endif