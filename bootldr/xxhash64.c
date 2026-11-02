/*
	Copyright (c) 2026 NorthernSturgeon
	SPDX-License-Identifier: MPL-2.0 AND MIT

	This Source Code Form is subject to the terms of the Mozilla Public 
	License, v. 2.0. If a copy of the MPL was not distributed with this
	file, You can obtain one at https://mozilla.org/MPL/2.0/.

	--------------------------------------------------------------------------------
	Portions derived from XXHash64 by Stephan Brumme:
	https://github.com/stbrumme/xxhash

	Copyright (c) 2016-2018 Stephan Brumme. All rights reserved.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
 */

#include "lib/types.h"
#include "xxhash64.h"

#define PRIME1 11400714785074694791ULL
#define PRIME2 14029467366897019727ULL
#define PRIME3 1609587929392839161ULL
#define PRIME4 9650029242287828579ULL
#define PRIME5 2870177450012600261ULL

static inline uint64_t rotl64(uint64_t x, int bits) {
	return (x << bits) | (x >> (64 - bits));
}

static inline uint64_t process_single(uint64_t previous, uint64_t input) {
	return rotl64(previous + input * PRIME2, 31) * PRIME1;
}

static inline uint32_t read32(const uint8_t* p){
	uint32_t val;
	__builtin_memcpy(&val, p, sizeof(val));
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	val = __builtin_bswap32(val);
#endif
	return val;
}

static inline uint64_t read64(const uint8_t* p){
	uint64_t val;
	__builtin_memcpy(&val, p, sizeof(val));
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	val = __builtin_bswap64(val);
#endif
	return val;
}

uint64_t xxhash64(const void* data, uint64_t length, uint64_t seed) {
	const uint8_t* p = (const uint8_t*)data;
	const uint8_t* const end = p + length;

	uint64_t v1 = seed + PRIME1 + PRIME2;
	uint64_t v2 = seed + PRIME2;
	uint64_t v3 = seed;
	uint64_t v4 = seed - PRIME1;

	while (p + 32 <= end) {
		v1 = process_single(v1, read64(p));
		v2 = process_single(v2, read64(p+8));
		v3 = process_single(v3, read64(p+16));
		v4 = process_single(v4, read64(p+24));
		p += 32;
	}

	uint64_t result;

	if (length >= 32) {
		result = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);
		result = (result ^ process_single(0, v1)) * PRIME1 + PRIME4;
		result = (result ^ process_single(0, v2)) * PRIME1 + PRIME4;
		result = (result ^ process_single(0, v3)) * PRIME1 + PRIME4;
		result = (result ^ process_single(0, v4)) * PRIME1 + PRIME4;
	} else {
		result = seed + PRIME5;
	}

	result += length;

	while (p + 8 <= end) {
		result = rotl64(result ^ process_single(0, read64(p)), 27) * PRIME1 + PRIME4;
		p += 8;
	}

	if (p + 4 <= end) {
		result = rotl64(result ^ (read32(p) * PRIME1), 23) * PRIME2 + PRIME3;
		p += 4;
	}

	while (p < end) {
		result = rotl64(result ^ (*p++ * PRIME5), 11) * PRIME1;
	}

	result ^= result >> 33;
	result *= PRIME2;
	result ^= result >> 29;
	result *= PRIME3;
	result ^= result >> 32;

	return result;
}