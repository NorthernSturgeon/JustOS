#ifndef __ARENA_H__
#define __ARENA_H__

#define ARENA_RESOLUTION 16

#define ARENA_SIZE 1048576
#define ARENA_THRESHOLD 65536

#define RESERVED_SIZE (ARENA_SIZE>>4) // Actually, minimal block size is 32 bytes

typedef uint16_t alist_t;
#define ALIST_MAX 65535

/*
if (ARENA_SIZE/ARENA_RESOLUTION > ALIST_MAX) new_arena->list[1] = ALIST_MAX;
else new_arena->list[1] = (ARENA_SIZE/ARENA_RESOLUTION);
*/
#define NEW_ARENA_EOL (ARENA_SIZE/ARENA_RESOLUTION > ALIST_MAX ? ALIST_MAX : ARENA_SIZE/ARENA_RESOLUTION)
#define NEW_ARENA_SOL (RESERVED_SIZE/ARENA_RESOLUTION)
#define NEW_ARENA_FREESIZE (NEW_ARENA_EOL - NEW_ARENA_SOL)

typedef struct __packed AllocatorArena{
	struct AllocatorArena *next;
	size_t max_free_size;
	size_t **pending;
	size_t lenght;
	alist_t flags; // guarantee of correct alignment
	alist_t list[];
} AllocatorArena;

//extern AllocatorArena* create_arena();
extern void* arena_allocate(size_t size);
//extern void arena_gc(AllocatorArena *arena);
extern void arena_free(AllocatorArena *arena, void* ptr);

#endif
