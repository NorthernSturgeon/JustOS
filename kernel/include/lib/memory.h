#ifndef __MEMORY_H__
#define __MEMORY_H__

#define memmask 0xFFFF800000000000ull

#define phys_to_virt(ptr) ptr | memmask
#define virt_to_phys(ptr) ptr & ~memmask

#endif