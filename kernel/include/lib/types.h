#ifndef __TYPES_H__
#define __TYPES_H__

typedef signed char int8_t;
typedef unsigned char uint8_t;

typedef signed short int16_t;
typedef unsigned short uint16_t;

typedef signed int int32_t;
typedef unsigned int uint32_t;

typedef signed long int64_t;
typedef unsigned long uint64_t;

typedef uint64_t size_t;

#define NULL ((void*)0)
#define TRUE ((uint8_t)1)
#define FALSE ((uint8_t)0)

#define __always_inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))
#define __packed __attribute__((__packed__))

#endif