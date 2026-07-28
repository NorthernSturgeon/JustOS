#include <efi/efi.h>
#include <efi/efilib.h>
#include "xxhash64.h"
#include "ldrerr.h"
#include "hashtab.h"

extern _Noreturn VOID Fatality(UINT32 code, UINT32 subcode); //from bootloader.c

#define SEED 17362530560801976147ull

static inline uint64_t hashstr(const char* x){
	return xxhash64(x, strlena(x), SEED);
}

static inline ht_entry_t* index(ht_entry_t *table, size_t i, size_t ts){
	return (ht_entry_t*)((uint8_t*)table + ts*i);
}

static inline ht_entry_t* ht_index(hashtab_t *tab, size_t i){
	return index(tab->table, i, tab->totalsize);
}

void ht_init(hashtab_t *tab, size_t data_size, size_t init_cap){
	tab->table = NULL;
	tab->capacity = 0;
	tab->entries = 0;
	tab->totalsize = data_size + sizeof(ht_entry_t);
	ht_realloc(tab, init_cap);
}

void ht_realloc(hashtab_t *tab, size_t new_cap){
	if (new_cap <= tab->entries) return;
	ht_entry_t *new_tab = AllocateZeroPool(new_cap*tab->totalsize);
	if (!new_tab) {
		ht_destroy(tab);
		Fatality(LDR_MEM, 1);
	}

	// rehash
	for (size_t i = 0; i < tab->capacity; i++){
		ht_entry_t *ptr_old = ht_index(tab,i), *ptr;
		if (ptr_old->key){
			uint64_t hash = ptr_old->fullhash;
			uint64_t h = hash % new_cap;

			while ((ptr = index(new_tab,h,tab->totalsize))->key) h = (h + 1) % new_cap;

			CopyMem(ptr, ptr_old, tab->totalsize);
		}
	}

	if (tab->table) FreePool(tab->table);

	tab->table = new_tab;
	tab->capacity = new_cap;
}

void* ht_insert(hashtab_t* restrict tab, const char* restrict key, void* restrict data){
	if (tab->capacity < 2) ht_realloc(tab, 2);
	else if (tab->entries*100 / tab->capacity >= 50) ht_realloc(tab, tab->capacity*2);

	uint64_t hash = hashstr(key);
	uint64_t h = hash % tab->capacity;

	ht_entry_t *ptr;
	while ((ptr = ht_index(tab,h))->key) h = (h + 1) % tab->capacity;
	
	ptr->key = key;
	ptr->fullhash = hash;

	void* ret = (uint8_t*)ptr + sizeof(ht_entry_t);
	if (data) CopyMem(ret, data, tab->totalsize - sizeof(ht_entry_t));

	tab->entries++;
	return ret;
}

void* ht_read(hashtab_t* restrict tab, const char* restrict name){
	if (!tab->capacity) return NULL;
	uint64_t hash = hashstr(name);
	uint64_t h = hash % tab->capacity;
	uint64_t h0 = h;

	ht_entry_t *ptr;
	while ((ptr = ht_index(tab,h))->key) {
		if (ptr->fullhash == hash && !strcmpa(ptr->key, name)) return (uint8_t*)ptr + sizeof(ht_entry_t);
		h = (h + 1) % tab->capacity;
		if (h == h0) return NULL;
	}

	return NULL;
}

void ht_destroy(hashtab_t *tab){
	if (tab->table) FreePool(tab->table);
	tab->table = (void*)BAD_POINTER; //fool-proofing: use-after-free will be seen via CR2 when #PF occurs
}