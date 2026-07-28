#ifndef __BL_HASHTAB_H__
#define __BL_HASHTAB_H__

typedef struct {
	const char* key;
	uint64_t fullhash;
} ht_entry_t;

typedef struct {
	ht_entry_t *table;
	size_t entries;
	size_t capacity;
	size_t totalsize;
} hashtab_t;

extern void ht_init(hashtab_t *tab, size_t data_size, size_t init_cap);
extern void ht_realloc(hashtab_t *tab, size_t new_cap);
extern void* ht_insert(hashtab_t* restrict tab, const char* restrict key, void* restrict data);
extern void* ht_read(hashtab_t* restrict tab, const char* restrict name);
extern void ht_destroy(hashtab_t *tab);

#endif