/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef __BL_HASHTAB_H__
#define __BL_HASHTAB_H__

/**
 * \file hashtab.h
 */

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

extern ht_entry_t* ht_index(hashtab_t *tab, size_t i);

#endif