#include "lib/arena.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "tls.h"
#include "physmem.h"
#include "interrupts.h"
#include <stdatomic.h>

//from lib/memory.c
extern uint64_t *size_table;
#define ST_IDX(addr) ((uint64_t)virt_to_phys(addr)>>12)

__tls AllocatorArena *cpu_top_arena = NULL;

/*
	TODO: optimal data structure for exclusive access
*/

static void optimize_arena(AllocatorArena *arena){
	size_t j = 0;
	for (size_t i = 0; i < arena->lenght; i++){
		if (i + 1 < arena->lenght && arena->list[i] == arena->list[i+1])
			i++;
		else
			arena->list[j++] = arena->list[i];
	}
	arena->lenght = j;
}

static uint64_t lowerbound(AllocatorArena *arena, alist_t start){
	int64_t l = -1, r = (int64_t)arena->lenght;

	while (r - l > 1){
		int64_t m = (r + l) / 2;
		if (arena->list[m] < start) l = m;
		else r = m;
	}

	return (uint64_t)r;
}

static uint64_t upperbound(AllocatorArena *arena, alist_t end, int64_t l0){
	int64_t l = l0, r = (int64_t)arena->lenght;

	while (r - l > 1){
		int64_t m = (r + l) / 2;
		if (arena->list[m] <= end) l = m;
		else r = m;
	}

	return (uint64_t)r;
}

static size_t arena_recalc(AllocatorArena *arena){
	alist_t max = 0;

	for (size_t i = 0; i < arena->lenght; i += 2){
		alist_t diff = arena->list[i+1] - arena->list[i];
		if (diff > max){
			max = diff;
		}
	}

	arena->max_available_block_size = max;
	return (size_t)max;
}

static void mark_free(AllocatorArena *arena, alist_t start, alist_t end){
	// remove zero-lenght
	optimize_arena(arena);

	if (arena->list[arena->lenght-1] < start){
		arena->list[arena->lenght++] = start;
		arena->list[arena->lenght++] = end;
	} else {
		//insert based on start and fix left border
		size_t si = lowerbound(arena, start); //start index

		if (si&1){ // si odd [F(si-1)<start, Bsi >= start]
			memmove(&arena->list[si+1], &arena->list[si-1], (arena->lenght-si+1)*sizeof(alist_t));
			// si odd [F(si-1) < start, Im F=start, Bsi=end, F(si+1)>=start (possibly wba)]
			si--;
			// si even [Fsi < start, Im F=start, B(si+1)=end, F(si+2)>=start (possibly wba)]
		} else { // si even [B(si-1)<start, Fsi >= start]
			memmove(&arena->list[si+2], &arena->list[si], (arena->lenght-si)*sizeof(alist_t));
			arena->list[si] = start;
			// si even [B(si-1)<start, Fsi=start, B(si+1)=end, F(si+2)>=start (possibly wba)]
		}
		arena->list[++si] = end; // Bsi=end (always odd)
		arena->lenght += 2;

		// absorb and fix
		size_t ei = upperbound(arena, end, si); //end index
		// ei odd [F(si-1), Bsi=end (will be erased), ..., F(ei-1)<=end, Bei>end] => ok!
		if (!(ei&1)){ // ei even [F(si-1), Bsi=end (will be erased), ..., Fei>end] -> save Bsi
			si++; 
			// [F(si-1), Bsi=end, si..., Fei>end]
		}

		// remove absorbed
		memmove(&arena->list[si], &arena->list[ei], (arena->lenght-ei)*sizeof(alist_t));
		arena->lenght = si + (arena->lenght-ei);
	}
};

void mark_busy(AllocatorArena *arena, alist_t start, alist_t end){
	size_t si = 0;
	// remove zero-lenght
	optimize_arena(arena);

	// must intersect with
	if (end > arena->list[0] && start < arena->list[arena->lenght-1]) {
		// special case: start below F(first) -> avoid i=0
		if (arena->list[0] >= start && end < arena->list[arena->lenght-1]) {
			arena->list[0] = end;
			goto absorb;
		// marking all memory as busy is prohibited
		} else if (!(start <= arena->list[0] && end >= arena->list[arena->lenght-1])) {
			//insert based on start and fix left border
			si = lowerbound(arena, start); //start index

			if (si&1){ // si odd [F(si-1)<start, Bsi >= start]
				memmove(&arena->list[si+2], &arena->list[si], (arena->lenght-si)*sizeof(alist_t));
				arena->list[si] = start;
				// si odd [F(si-1)<start, Bsi=start, F(si+1)=end, B(si+2)>=start (possibly wba)]
			} else { // si even [B(si-1)<start, Fsi >= start], unsafe if si=0
				memmove(&arena->list[si+1], &arena->list[si-1], (arena->lenght-si+1)*sizeof(alist_t));
				// si even [B(si-1) < start, Im B=start, Fsi=end, B(si+1)>=start (possibly wba)]
				si--;
				// si odd [Bsi < start, Im B=start, F(si+1)=end, B(si+2)>=start (possibly wba)]
			}
			arena->list[++si] = end; // Fsi=end (always even), never last element
			arena->lenght += 2;

			absorb:
			// absorb and fix
			size_t ei = upperbound(arena, end, si); //end index
			// ei even [B(si-1), Fsi=end (will be erased), ..., B(ei-1)<=end, Fei>end]
			if (ei&1){ // ei odd [B(si-1), Fsi=end (will be erased), ..., Bei>end] -> save Fsi
				si++;
				// [B(si-1), Fsi=end, si..., Bei>end]
			}

			// remove absorbed
			memmove(&arena->list[si], &arena->list[ei], (arena->lenght-ei)*sizeof(alist_t));
			arena->lenght = si + (arena->lenght-ei);

			// special case: B(last) <= end -> remove Fp=end
			arena->lenght -= arena->lenght&1;
		}
	}
}

static AllocatorArena* create_arena(){
	AllocatorArena *new_arena = allocate_pages(ARENA_SIZE/PAGE_SIZE);
	if (!new_arena) return NULL;

	new_arena->pending = NULL;
	new_arena->lenght = 2;
	new_arena->list[0] = RESERVED_SIZE/ARENA_RESOLUTION;

	/*
	if (ARENA_SIZE/ARENA_RESOLUTION > ALIST_MAX) new_arena->list[1] = ALIST_MAX;
	else new_arena->list[1] = (ARENA_SIZE/ARENA_RESOLUTION);
	*/
	new_arena->list[1] = ALIST_MAX;
	new_arena->max_available_block_size = new_arena->list[1] - new_arena->list[0];

	for (size_t i = 0; i < ARENA_SIZE; i += PAGE_SIZE){
		size_table[ST_IDX(((void*)new_arena) + i)] = (uint64_t)new_arena;
	}

	AllocatorArena **top_arena = tls_ptr(cpu_top_arena);
	new_arena->next = *top_arena;
	*top_arena = new_arena;

	return new_arena;
}

/*
	Restrictions:
	- disabled interrupts,
	- only current thread arenas
*/
static size_t arena_gc(AllocatorArena *arena){
	size_t *val = atomic_load_explicit(&arena->pending, __ATOMIC_ACQUIRE), *prev = (size_t*)&arena->pending;
	
	if (val){ 
		while (val) {
			atomic_store(prev, 0);
			alist_t start = ((uint8_t*)val - (uint8_t*)arena)/ARENA_RESOLUTION;
			size_t *next = (size_t*)atomic_load(val);
			mark_free(arena, start, start + *(val+1));
			prev = val;
			val = next;
		}

		return arena_recalc(arena);
	}
	return 0;
}

void* arena_allocate(size_t size){
	if (size >= ARENA_THRESHOLD) return NULL;
	size = round_to(size, ARENA_RESOLUTION) + ARENA_RESOLUTION;

	size /= ARENA_RESOLUTION;

	disable_irq();
retry:
	AllocatorArena *current = *tls_ptr(cpu_top_arena);

	if (!current || current->max_available_block_size < size) {
		if (current) {
			if (arena_gc(current) >= size) goto found;
		}
		current = create_arena();
		if (!current) {
			enable_irq();
			return NULL;
		}
	} else {
		AllocatorArena* next;
		while ((next = current->next)){
			if (next->max_available_block_size < size){
				if (arena_gc(next) < size) goto found;
			}
			current = next;
		}
	}
found:
	alist_t bestsize = ALIST_MAX;
	size_t best = SIZE_MAX;

	alist_t max1 = 0, max2 = 0;
	size_t imax1 = 0; //, imax2 = 0;
	for (size_t i = 0; i < current->lenght; i += 2){
		alist_t diff = current->list[i+1] - current->list[i];
		if (diff >= size && diff < bestsize) {
			best = i;
			bestsize = diff;
		}

		if (diff > max1){
			max2 = max1;
			//imax2 = imax1;

			max1 = diff;
			imax1 = i;
		} else if (diff > max2){
			max2 = diff;
			//imax2 = i;
		}
	}

	if (best == SIZE_MAX){
		current->max_available_block_size = max1;
		goto retry;
	}

	current->list[best+1] -= size;
	size_t* ptr = (void*)current + current->list[best+1]*ARENA_RESOLUTION;

	if (best == imax1) {
		current->max_available_block_size = (bestsize - size < max2) ? max2 : bestsize - size;
	}

	enable_irq();

	*ptr = 0;
	*++ptr = size;
	return ++ptr;
	// node creation happens-before
}

void arena_free(AllocatorArena *arena, void* ptr){
	ptr -= 16; // ptr is void*
	size_t *val = atomic_load_explicit(&arena->pending, __ATOMIC_ACQUIRE);

	do{
		*(size_t*)ptr = (size_t)val;
	} while (atomic_compare_exchange_strong(&arena->pending, &val, ptr));
}