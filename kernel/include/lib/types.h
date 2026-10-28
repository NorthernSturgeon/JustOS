#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define asm __asm__

#define __export __attribute__((visibility("default")))
#define __always_inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))
#define __packed __attribute__((__packed__))
#define __naked __attribute__((naked))

#define PAGE_SIZE 4096

#define OPT_BARRIER() asm volatile ("" ::: "memory")

#if defined(LOG_DEBUG)

#define log_debug(...) printf(__VA_ARGS__)
#define log_warn(...) printf(__VA_ARGS__)

#elif defined(LOG_WARN)

#define log_debug(...)
#define log_warn(...) printf(__VA_ARGS__)

#else

#define log_debug(...)
#define log_warn(...)

#endif

#endif