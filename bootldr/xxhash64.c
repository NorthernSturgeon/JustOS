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

uint64_t xxhash64(const void* data, uint64_t length, uint64_t seed) {
    const uint8_t* p = (const uint8_t*)data;
    const uint8_t* const end = p + length;

    uint64_t v1 = seed + PRIME1 + PRIME2;
    uint64_t v2 = seed + PRIME2;
    uint64_t v3 = seed;
    uint64_t v4 = seed - PRIME1;

    while (p + 32 <= end) {
        const uint64_t* block = (const uint64_t*)p;
        v1 = process_single(v1, block[0]);
        v2 = process_single(v2, block[1]);
        v3 = process_single(v3, block[2]);
        v4 = process_single(v4, block[3]);
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
        result = rotl64(result ^ process_single(0, *(const uint64_t*)p), 27) * PRIME1 + PRIME4;
        p += 8;
    }

    if (p + 4 <= end) {
        result = rotl64(result ^ (*(const uint32_t*)p * PRIME1), 23) * PRIME2 + PRIME3;
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