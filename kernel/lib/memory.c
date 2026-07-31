#include "physmem.h"

static size_t to_pages(size_t bytes){
	return (bytes>>12) + (bytes&0xfff ? 1 : 0);
}

void free(void* ptr){
	if (!ptr) return;

	size_t size = gorl.table[(uint64_t)virt_to_phys(ptr)>>12];
	free_pages(ptr, size);
}

void* malloc(size_t size){
    if (!size) return NULL;

    size = to_pages(size);
    void* ptr = allocate_pages(size);

    if (ptr) gorl.table[(uint64_t)virt_to_phys(ptr)>>12] = size;

    return ptr;
}