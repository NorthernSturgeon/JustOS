#include "lib/arena.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "tls.h"
#include "physmem.h"
#include <stdatomic.h>
#include "lib/console.h"

//from lib/memory.c
extern uint64_t *size_table;
#define ST_IDX(addr) ((uint64_t)virt_to_phys(addr)>>12)

__tls AllocatorArena *cpu_top_arena = NULL;
__tls size_t *cpu_pending = NULL;
__tls size_t cpu_arenas_cnt = 0;

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

	arena->max_free_size = max;
	arena->flags = 0;
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

	arena->flags = 1;
};

/*
static void mark_busy(AllocatorArena *arena, alist_t start, alist_t end){
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

	arena_recalc(arena);
}
*/
static AllocatorArena* create_arena(){
	AllocatorArena *new_arena = allocate_pages(ARENA_SIZE/PAGE_SIZE);
	log_debug("create_arena: %p old_cnt=%u\n", new_arena, cpu_arenas_cnt);
	if (!new_arena) return NULL;

	new_arena->pending = &cpu_pending;
	new_arena->lenght = 2;
	new_arena->list[0] = NEW_ARENA_SOL;
	new_arena->list[1] = NEW_ARENA_EOL;
	new_arena->max_free_size = NEW_ARENA_FREESIZE;
	new_arena->flags = 0;

	for (size_t i = 0; i < ARENA_SIZE; i += PAGE_SIZE){
		size_table[ST_IDX(((void*)new_arena) + i)] = (uint64_t)new_arena;
	}

	OPT_BARRIER();

	new_arena->next = cpu_top_arena;
	cpu_top_arena = new_arena;

	cpu_arenas_cnt += 1;

	return new_arena;
}

static void destroy_arena(AllocatorArena* arena, AllocatorArena* prev){
	log_debug("destroy_arena(%p)\n", arena);

	if (arena->max_free_size < NEW_ARENA_FREESIZE) {
		log_warn("[WARN] Prevention an attempt to destroy non-empty arena\n");
		return;
	}

	if (prev) prev->next = arena->next;
	else cpu_top_arena = arena->next;

	OPT_BARRIER();

	for (size_t i = 0; i < ARENA_SIZE; i += PAGE_SIZE){
		size_table[ST_IDX(((void*)arena) + i)] = 0;
	}

	free_pages(arena, ARENA_SIZE/PAGE_SIZE);
}

static AllocatorArena** merge(AllocatorArena* left, AllocatorArena* right, AllocatorArena** target){
	while (left && right){
		if (left->max_free_size >= right->max_free_size){
			*target = left;
			left = left->next;
		} else {
			*target = right;
			right = right->next;
		}

		target = &((*target)->next);
	}
	
	*target = left ? left : right;

    while (*target) {
        target = &((*target)->next);
    }

	return target;
}

static AllocatorArena* split(AllocatorArena* start, size_t n){
	if (!start) return NULL;

	for (size_t i = 1; i < n && start->next; i++){
		start = start->next;
	}

	AllocatorArena* target = start->next;
	start->next = NULL;
	return target;
}

// iterative merge sort (reversed)
static void sort_and_update(){
	size_t lenght = cpu_arenas_cnt;
	if (lenght < 2) return;

	AllocatorArena *head, **tail;

	for (size_t step = 1; step < lenght; step *= 2){
		tail = &cpu_top_arena;
		head = *tail;

		while (head){
			AllocatorArena *left = head;
			AllocatorArena *right = split(left, step);

			head = split(right, step);

			tail = merge(left, right, tail);
		}
		
	}
}

static void sift_down(AllocatorArena *arena, AllocatorArena *prev){
	size_t counter = 0;
	// so prev->next is arena
	for (
		AllocatorArena *target = arena->next;
		target && target->max_free_size > arena->max_free_size; 
		target = arena->next
	) {
		arena->next = target->next;

		target->next = arena;

		if (prev) prev->next = target;
		else cpu_top_arena = target;

		prev = target;
		counter++;
	}
	if (counter) {log_debug("sift_down: %u times\n", counter); }
}

static bool recalc_and_check(){
	bool seen_empty = false;
	size_t unsorted = 0;

	AllocatorArena *current = cpu_top_arena, *prev = NULL;

	while (current){
		log_debug("recalc on arena %p \n", current);
		if (current->flags) arena_recalc(current);

		if (current->max_free_size == NEW_ARENA_FREESIZE){
			if (seen_empty) {
				// seen_empty is true => prev is not NULL
				destroy_arena(current, prev);
				current = prev->next;
				continue;
			}
			else seen_empty = true;
		}

		if (prev && prev->max_free_size < current->max_free_size){
			unsorted++;
		}

		prev = current;
		current = current->next;
	}

	log_debug("recalc_and_check: %u ", unsorted);
	return (bool)unsorted;
}

static void aalloc_gc(){
	log_debug("aalloc_gc(): ");
	size_t *val = NULL, *prev = (size_t*)&cpu_pending, counter = 0;
	val = (size_t*)atomic_exchange_explicit(prev, (size_t)val, __ATOMIC_ACQUIRE);
	AllocatorArena *arena;
	
	if (val){
		// process pending
		while (val) {
			arena = (AllocatorArena*)size_table[ST_IDX(val)];
			alist_t start = ((uint8_t*)val - (uint8_t*)arena)/ARENA_RESOLUTION;
			size_t *next = (size_t*)*val;
			// log_debug("    lazy_free(%u, %u) @ %p\n", (size_t)start*ARENA_RESOLUTION, *(val+1), arena);
			mark_free(arena, start, start + *(val+1)); //little-endian only!
			val = next;
			counter++;
		}
		// sort arenas
		if (recalc_and_check()) sort_and_update();
	}
	log_debug("freed %u entries\n", counter);
}

void* arena_allocate(size_t size){
	if (size >= ARENA_THRESHOLD) return NULL;
	size = round_to(size, ARENA_RESOLUTION) + ARENA_RESOLUTION;

	size /= ARENA_RESOLUTION;

	preempt_disable();
retry:
	AllocatorArena *current = cpu_top_arena, *prev = NULL;

	if (!current || current->max_free_size < size) {
		if (current) {
			aalloc_gc();
			current = cpu_top_arena;
			if (current->max_free_size >= size) goto find_best;
		}
		current = create_arena();
		if (!current) {
			preempt_enable();
			return NULL;
		}
	} else { find_best:
		AllocatorArena* next;
		while ((next = current->next)){
			if (next->max_free_size < size) goto found;

			prev = current;
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
		log_warn("[WARN] arena->max_free_size desynchronization\n");
		current->max_free_size = max1;
		sift_down(current, prev);
		goto retry;
	}

	current->list[best+1] -= size;
	size_t* ptr = (void*)current + current->list[best+1]*ARENA_RESOLUTION;

	if (best == imax1) {
		current->max_free_size = (bestsize - size < max2) ? max2 : bestsize - size;

		sift_down(current, prev);
	}

	preempt_enable();

	log_debug("arena_allocate(%u): arena %p offset %u\n", size, current, (void*)ptr - (void*)current);

	*ptr = 0;
	*++ptr = size;
	return ++ptr;
}

void arena_free(AllocatorArena *arena, void* ptr){
	ptr = (void*)(((uint64_t)ptr) - 16); // ptr is void*
	// log_debug("arena_free(): arena %p offset %u\n", arena, (void*)ptr - (void*)arena);

	size_t *val = atomic_load_explicit(arena->pending, __ATOMIC_RELAXED);

	do{
		*(size_t*)ptr = (size_t)val;
	} while (atomic_compare_exchange_weak_explicit(arena->pending, &val, ptr, __ATOMIC_RELEASE, __ATOMIC_RELAXED));
}