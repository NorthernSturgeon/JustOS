#include "lib/string.h"
#include "lib/atomic.h"
#include "lib/console.h"
#include <alloca.h>
#include "physmem.h"
#include "boot.h"

#define MARKER __UINT64_MAX__

__export gorl_t gorl;

__export e820_entry_t *e820;
__export size_t e820_len;

static uint64_t round_to(uint64_t n, uint64_t k){
	return n + (n%k ? k - (n%k) : 0);
}

static size_t to_pages(size_t bytes){
	return (bytes>>12) + (bytes&0xfff ? 1 : 0);
}

// call under lock!
void optimize_gorl(){
	size_t j = 0;
	for (size_t i = 0; i < gorl.lenght; i++){
		if (i + 1 < gorl.lenght && gorl.list[i] == gorl.list[i+1])
			i++;
		else
			gorl.list[j++] = gorl.list[i];
	}
	gorl.lenght = j;
}

//size in bytes
void mark_free(void* ptr, size_t size){
	uint64_t start = (uint64_t)ptr;
	uint64_t end = start+size;
	size_t place = 0, j = 0;

	rwlock_lock(&gorl.rwlock);
	// remove zero-lenght
	optimize_gorl();

	if (gorl.list[gorl.lenght-1] < start){
		gorl.list[gorl.lenght++] = start;
		gorl.list[gorl.lenght++] = end;
	} else {
		//insert based on start and fix left border
		for (size_t i = 0; i < gorl.lenght; i++){
			if (gorl.list[i] >= start){
				if (i&1){ // i odd [F(i-1)<start, Bi >= start]
					memmove(&gorl.list[i+1], &gorl.list[i-1], (gorl.lenght-i+1)*sizeof(uint64_t));
					// i odd [F(i-1) < start, Im F=start, Bi=end, F(i+1)>=start (possibly wba)]
					i--;
					// i even [Fi < start, Im F=start, B(i+1)=end, F(i+2)>=start (possibly wba)]
				} else { // i even [B(i-1)<start, Fi >= start]
					memmove(&gorl.list[i+2], &gorl.list[i], (gorl.lenght-i)*sizeof(uint64_t));
					gorl.list[i] = start;
					// i even [B(i-1)<start, Fi=start, B(i+1)=end, F(i+2)>=start (possibly wba)]
				}
				gorl.list[++i] = end;
				gorl.lenght += 2;
				place = i; // Bp=end (always odd)
				break;
			}
		}

		// absorb and fix right border
		for (size_t i = place + 1; i < gorl.lenght; i++){
			if (gorl.list[i] > end){
				// i even [F(p-1), Bp=end, ..., Fi>end] => ok!
				if (i&1){ // i odd [F(p-1), Bp=end, ..., F(i-1)<=end (erased), Bi>end]
					gorl.list[place] = MARKER;
					// [F(p-1), Bp=end (erased), ..., F(i-1)<=end (erased), Bi>end]
				}
				break;
			}
			//gorl.list[i] <= end -> absorb
			gorl.list[i] = MARKER;
		}

		// remove marked
		for (size_t i = 0; i < gorl.lenght; i++) {
			if (gorl.list[i] != MARKER)
				gorl.list[j++] = gorl.list[i];
		}
		gorl.lenght = j;
	}

	rwlock_unlock(&gorl.rwlock);
};

//size in bytes
void mark_busy(void* ptr, size_t size){
	uint64_t start = (uint64_t)ptr;
	uint64_t end = start+size;
	size_t place = 0, j = 0;

	rwlock_lock(&gorl.rwlock);
	// remove zero-lenght
	optimize_gorl();

	// must intersect with
	if (end > gorl.list[0] && start < gorl.list[gorl.lenght-1]) {
		// special case: start below F(first) -> avoid i=0
		if (gorl.list[0] >= start && end < gorl.list[gorl.lenght-1]) {
			gorl.list[0] = end;
			goto absorb;
		// marking all memory as busy is prohibited
		} else if (!(start <= gorl.list[0] && end >= gorl.list[gorl.lenght-1])) {
			//insert based on start and fix left border
			for (size_t i = 1; i < gorl.lenght; i++){
				if (gorl.list[i] >= start){
					if (i&1){ // i odd [F(i-1)<start, Bi >= start]
						memmove(&gorl.list[i+2], &gorl.list[i], (gorl.lenght-i)*sizeof(uint64_t));
						gorl.list[i] = start;
						// i odd [F(i-1)<start, Bi=start, F(i+1)=end, B(i+2)>=start (possibly wba)]
					} else { // i even [B(i-1)<start, Fi >= start], unsafe if i=0
						memmove(&gorl.list[i+1], &gorl.list[i-1], (gorl.lenght-i+1)*sizeof(uint64_t));
						// i even [B(i-1) < start, Im B=start, Fi=end, B(i+1)>=start (possibly wba)]
						i--;
						// i odd [Bi < start, Im B=start, F(i+1)=end, B(i+2)>=start (possibly wba)]
					}
					gorl.list[++i] = end;
					gorl.lenght += 2;
					place = i; // Fp=end (always even), never last element
					break;
				}
			}

			absorb:
			// absorb and fix right border
			for (size_t i = place + 1; i < gorl.lenght; i++){
				if (gorl.list[i] > end){
					// i odd [B(p-1), Fp=end, ..., Bi>end] => ok!
					if (!(i&1)){ // i even [B(p-1), Fp=end, ..., B(i-1)<=end (erased), Fi>end]
						gorl.list[place] = MARKER;
						// [B(p-1), Fp=end (erased), ..., B(i-1)<=end (erased), Fi>end]
					}
					break;
				}
				//gorl.list[i] <= end -> absorb
				gorl.list[i] = MARKER;
			}

			// remove marked
			for (size_t i = 0; i < gorl.lenght; i++) {
				if (gorl.list[i] != MARKER)
					gorl.list[j++] = gorl.list[i];
			}
			gorl.lenght = j;

			// special case: B(last) <= end -> remove Fp=end
			// lenght must be even
			gorl.lenght -= gorl.lenght&1;
		}
	}

	rwlock_unlock(&gorl.rwlock);
};

typedef struct __packed{
	uint64_t key:48;
	uint64_t value:16;
} heapq_t;

//min-heap
static inline void heapify(heapq_t *heap, size_t i, size_t size){
	heapq_t current = heap[i];
	size_t child;
	
	while((child = 2*i + 1) < size){
		if (child + 1 < size && heap[child+1].key < heap[child].key) child++;
		if (!(heap[child].key < current.key)) break;

		heap[i] = heap[child];
		i = child;
	}
	heap[i] = current;
}

static inline uint64_t get_min(heapq_t *heap, size_t *size){
	uint64_t ret = heap[0].value;
	heap[0] = heap[--(*size)];
	heapify(heap, 0, *size);
	return ret;
}

//size in pages
__export void* allocate_pages(size_t size){
	size <<= 12;
	uint64_t result;

	rwlock_inc(&gorl.rwlock);

	size_t heap_size = gorl.lenght/2;
	heapq_t *heapq = alloca(heap_size*sizeof(heapq_t));

	for (size_t i = 0; i < gorl.lenght; i += 2){
		heapq[i/2].value = i;
		heapq[i/2].key = 
			atomic_load_explicit(&gorl.list[i+1], __ATOMIC_RELAXED) - atomic_load_explicit(&gorl.list[i], __ATOMIC_RELAXED);
	}

	for(size_t i = heap_size/2; i > 0;){
		heapify(heapq, --i, heap_size);
	}

	while (heap_size){
		size_t i = get_min(heapq, &heap_size);
		do {
			result = atomic_load(&gorl.list[i]);
			if (atomic_load(&gorl.list[i+1]) < result+size) goto next;
		} while(!atomic_compare_exchange_weak(&gorl.list[i], &result, result+size));
		goto exit;
		next:
	}
	result = (uint64_t)NULL;
exit:
	rwlock_dec(&gorl.rwlock);
	return (void*)result;
}

//size in pages
__export void free_pages(void* ptr, size_t size){
	size <<= 12;

	rwlock_inc(&gorl.rwlock);
	for (size_t i = 0; i < gorl.lenght; i += 2){
		uint64_t lower = (uint64_t)ptr;
		uint64_t upper = lower + size;
		if((atomic_load(&gorl.list[i+1]) == lower && atomic_compare_exchange_strong(&gorl.list[i+1], &lower, upper)) \
		|| (atomic_load(&gorl.list[i]) == upper && atomic_compare_exchange_strong(&gorl.list[i], &upper, (uint64_t)ptr))) goto success;
	}
// end of list reached - region in middle
	rwlock_dec(&gorl.rwlock);
	mark_free(ptr, size);
	return;
success:
	rwlock_dec(&gorl.rwlock);
}

void init_mm(){
	e820 = boot_info->mmap;
	e820_len = boot_info->mmap_len;

	//sort memory map
	for (int64_t i = 1; i < (int64_t)e820_len; i++){
		e820_entry_t temp = e820[i];
		int64_t j = i-1;
		for (;j >= 0 && e820[j].base > temp.base; j--) e820[j+1] = e820[j];
		e820[j+1] = temp;
	}

	//merge same types
	uint64_t newsize = 0;
	for (uint64_t i = 1; i < e820_len; i++){
		if (e820[i].type == e820[newsize].type && e820[i].attr == e820[newsize].attr){
			e820[newsize].lenght += e820[i].lenght;
		} else {
			newsize++;
			e820[newsize] = e820[i];
		}
	}
	e820_len = newsize + 1;

	gorl.list = (uint64_t*)(boot_info->mmap + boot_info->mmap_len);
	gorl.rwlock = 0;

	size_t gorl_size = 0;

	for (uint64_t i = 0; i < e820_len; i++){
		if (e820[i].type == E820_TYPE_USABLE){
			gorl.list[gorl_size++] = (uint64_t)phys_to_virt(e820[i].base);
			gorl.list[gorl_size++] = (uint64_t)phys_to_virt(e820[i].base) + e820[i].lenght;
		}
	}

	gorl.lenght = gorl_size;

	size_t total = (size_t)&gorl.list[gorl.lenght] - (size_t)boot_info->ptzone;

	//
	mark_busy(phys_to_virt(NULL), 1u<<20); // first megabyte
	mark_busy(boot_info->ptzone, round_to(total, 4096)); // page tables and after

	printf("init_mm: file_table %p count %u\n", boot_info->files, boot_info->files_cnt);
	mark_busy(boot_info->files, round_to(boot_info->files_cnt*sizeof(loaded_file), 4096));
	for (size_t i = 0; i < boot_info->files_cnt; i++){
		loaded_file *file = &boot_info->files[i];
		printf("init_mm: file %s @ %p / %u b, TLS@%p / %u b\n", file->name, file->data, file->size, file->tls_area, file->tls_size);
		mark_busy(file->data, round_to(file->size, 4096));
		if (file->tls_area)	mark_busy(file->tls_area, round_to(file->tls_size, 4096));
	}

	mark_busy(boot_info->stack, boot_info->stack_size); // kernel stack

	size_t st_size = to_pages((size_t)virt_to_phys(gorl.list[gorl.lenght-1]) >> 9);
	gorl.table = allocate_pages(st_size);

	printf("init_mm: size table %p, %u pages, %u kB\n", gorl.table, st_size, st_size << 2);

	//NULLPTR!
	if (!gorl.table){
		printf("FATAL: not enough memory for size table\n");
		for(;;);
	}

	gorl.table[(uint64_t)virt_to_phys(gorl.table)>>12] = st_size;
}