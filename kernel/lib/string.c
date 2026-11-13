/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include <immintrin.h>

#define LAZY_ALIGN 16

size_t strlen(const char *str){
	size_t len = 0;

	uint16_t lazy = ((uintptr_t)str) & 0xFFF;
	if (lazy) lazy = 4096 - lazy;

	if (lazy > LAZY_ALIGN) {
		while (len < LAZY_ALIGN){
			int mask = _mm_movemask_epi8(
				_mm_cmpeq_epi8(
					_mm_loadu_si128((__m128i*)str), // unaligned load
					_mm_setzero_si128()
				)
			);

			if (mask) return len + __builtin_ctz(mask);

			len += 16;
			str += 16;
		}
	}

	uint8_t to_align = ((uintptr_t)str) & 0xF;
	if (to_align) to_align = 16 - to_align;

	while (to_align) {
		if (*str++ == '\0') return len;
		len++;
		to_align--;
	}

	while (1){
		int mask = _mm_movemask_epi8(
			_mm_cmpeq_epi8(
				_mm_load_si128((__m128i*)str), // aligned load
				_mm_setzero_si128()
			)
		);

		if (mask) return len + __builtin_ctz(mask);

		len += 16;
		str += 16;
	}
}
