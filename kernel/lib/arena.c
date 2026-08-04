#include "lib/arena.h"
#include "physmem.h"
#include "interrupts.h"

struct arena{
	struct arena *next;
	size_t free_size;
	uint16_t list[];
};

