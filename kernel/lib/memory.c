#include "physmem.h"
#include "lib/console.h"

static size_t to_pages(size_t bytes){
	return (bytes>>12) + (bytes&0xfff ? 1 : 0);
}

static uint64_t *size_table = NULL;

void init_libcalloc(void* max_address){
	if (size_table) return;

	size_t st_size = to_pages((size_t)max_address >> 9);
	size_table = allocate_pages(st_size);

	printf("libc: size table %p, %u pages, %u kB\n", size_table, st_size, st_size << 2);

	//NULLPTR!
	if (!size_table){
		printf("FATAL: not enough memory for size table\n");
		for(;;);
	}

	size_table[(uint64_t)virt_to_phys(size_table)>>12] = st_size;
}

void free(void* ptr){
	if (!ptr) return;

	size_t size = size_table[(uint64_t)virt_to_phys(ptr)>>12];
	free_pages(ptr, size);
}

void* malloc(size_t size){
    if (!size) return NULL;

    size = to_pages(size);
    void* ptr = allocate_pages(size);

    if (ptr) size_table[(uint64_t)virt_to_phys(ptr)>>12] = size;

    return ptr;
}