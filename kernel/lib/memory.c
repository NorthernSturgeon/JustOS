#include "physmem.h"
#include "lib/arena.h"
#include "lib/console.h"

static size_t to_pages(size_t bytes){
	return (bytes>>12) + (bytes&0xfff ? 1 : 0);
}

uint64_t round_to(uint64_t n, uint64_t k){
	return n + (n%k ? k - (n%k) : 0);
}

uint64_t *size_table = NULL;
#define ST_IDX(addr) ((uint64_t)virt_to_phys(addr)>>12)

__export void init_malloc(void* max_address){
	if (size_table) return;

	size_t st_size = to_pages((size_t)max_address >> 9);
	size_table = allocate_pages(st_size);

	printf("klib: size table %p, %u pages, %u kB\n", size_table, st_size, st_size << 2);

	//NULLPTR!
	if (!size_table){
		printf("FATAL: not enough memory for size table\n");
		for(;;);
	}

	size_table[ST_IDX(size_table)] = st_size;
}

__export void free(void* ptr){
	if (!ptr) return;

	size_t size = size_table[ST_IDX(ptr)];
	if (!size) return; //TODO: panic

	if ((intptr_t)size > 0) free_pages(ptr, size); //size
	else arena_free((AllocatorArena*)size, ptr); //virtual address
}

__export void* malloc(size_t size){
    if (!size) return NULL;
	if (size < ARENA_THRESHOLD) return arena_allocate(size);

    size = to_pages(size);
    void* ptr = allocate_pages(size);

    if (ptr) size_table[ST_IDX(ptr)] = size;

    return ptr;
}