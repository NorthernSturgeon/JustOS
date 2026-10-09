#ifndef __ARENA_H__
#define __ARENA_H__

#define ARENA_RESOLUTION 16

#define ARENA_SIZE 1048576
#define ARENA_THRESHOLD 65536

#define RESERVED_SIZE (ARENA_SIZE>>4)

typedef uint16_t alist_t;
#define ALIST_MAX 65535

typedef struct __packed AllocatorArena{
	struct AllocatorArena *next;
	size_t max_available_block_size;
	void *pending;
	size_t lenght;
	alist_t list[];
} AllocatorArena;

//extern AllocatorArena* create_arena();
extern void* arena_allocate(size_t size);
//extern void arena_gc(AllocatorArena *arena);
extern void arena_free(AllocatorArena *arena, void* ptr);

#endif
