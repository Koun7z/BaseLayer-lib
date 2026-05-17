#include "Base_Hash.h"
#include "time.h"

static inline uint64_t rotl64(uint64_t x, int r)
{
    return (x << r) | (x >> (64 - r));
}

static inline uint32_t rotl32(uint32_t x, int r)
{
    return (x << r) | (x >> (32 - r));
}

uint32_t Hash_Murmur3_u32(const void* key, size_t len, uint32_t seed)
{
    const uint8_t* data = (const uint8_t*) key;
    const int nblocks   = len / 4;

    uint32_t h = seed;

    const uint32_t c1 = UINT32_C(0xcc9e2d51);
    const uint32_t c2 = UINT32_C(0x1b873593);

    const uint32_t* blocks = (const uint32_t*) (data + nblocks * 4);
    for(int i = -nblocks; i; i++)
    {
        uint32_t k = blocks[i];

        k *= c1;
        k  = rotl32(k, 15);
        k *= c2;

        h ^= k;
        h  = rotl32(h, 13);
        h  = h * 5 + UINT32_C(0xe6546b64);
    }

    const uint8_t* tail = data + nblocks * 4;
    uint32_t k1         = 0;

    switch(len & 3)
    {
        case 3:
            k1 ^= tail[2] << 16;
            /* fallthrough */
        case 2:
            k1 ^= tail[1] << 8;
            /* fallthrough */
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1  = rotl32(k1, 15);
            k1 *= c2;
            h  ^= k1;
    }

    h ^= len;
    h ^= h >> 16;
    h *= UINT32_C(0x85ebca6b);
    h ^= h >> 13;
    h *= UINT32_C(0xc2b2ae35);
    h ^= h >> 16;

    return h;
}

uint64_t Hash_Murmur3_u64(const void* key, size_t len, uint64_t seed)
{
    const uint8_t* data = (const uint8_t*) key;
    const int nblocks   = len / 16;

    uint64_t h1 = seed;
    uint64_t h2 = seed;

    const uint64_t c1 = 0x87c37b91114253d5ULL;
    const uint64_t c2 = 0x4cf5ad432745937fULL;

    const uint64_t* blocks = (const uint64_t*) data;
    for(int i = 0; i < nblocks; i++)
    {
        uint64_t k1 = blocks[i * 2 + 0];
        uint64_t k2 = blocks[i * 2 + 1];

        k1 *= c1;
        k1  = rotl64(k1, 31);
        k1 *= c2;
        h1 ^= k1;
        h1  = rotl64(h1, 27);
        h1 += h2;
        h1  = h1 * 5 + UINT64_C(0x52dce729);

        k2 *= c2;
        k2  = rotl64(k2, 33);
        k2 *= c1;
        h2 ^= k2;
        h2  = rotl64(h2, 31);
        h2 += h1;
        h2  = h2 * 5 + UINT64_C(0x38495ab5);
    }

    const uint8_t* tail = data + nblocks * 16;
    uint64_t k1         = 0;
    uint64_t k2         = 0;

    switch(len & 15)
    {
        case 15:
            k2 ^= (uint64_t) tail[14] << 48;
            /* fall through */
        case 14:
            k2 ^= (uint64_t) tail[13] << 40;
            /* fall through */
        case 13:
            k2 ^= (uint64_t) tail[12] << 32;
            /* fall through */
        case 12:
            k2 ^= (uint64_t) tail[11] << 24;
            /* fall through */
        case 11:
            k2 ^= (uint64_t) tail[10] << 16;
            /* fall through */
        case 10:
            k2 ^= (uint64_t) tail[9] << 8;
            /* fall through */
        case 9:
            k2 ^= (uint64_t) tail[8];
            k2 *= c2;
            k2  = rotl64(k2, 33);
            k2 *= c1;
            h2 ^= k2;
            /* fall through */
        case 8:
            k1 ^= (uint64_t) tail[7] << 56;
            /* fall through */
        case 7:
            k1 ^= (uint64_t) tail[6] << 48;
            /* fall through */
        case 6:
            k1 ^= (uint64_t) tail[5] << 40;
            /* fall through */
        case 5:
            k1 ^= (uint64_t) tail[4] << 32;
            /* fall through */
        case 4:
            k1 ^= (uint64_t) tail[3] << 24;
            /* fall through */
        case 3:
            k1 ^= (uint64_t) tail[2] << 16;
            /* fall through */
        case 2:
            k1 ^= (uint64_t) tail[1] << 8;
            /* fall through */
        case 1:
            k1 ^= (uint64_t) tail[0];
            k1 *= c1;
            k1  = rotl64(k1, 31);
            k1 *= c2;
            h1 ^= k1;
    }

    h1 ^= len;
    h2 ^= len;
    h1 += h2;
    h2 += h1;

    h1 ^= h1 >> 33;
    h1 *= UINT64_C(0xff51afd7ed558ccd);
    h1 ^= h1 >> 33;
    h2 ^= h2 >> 33;
    h2 *= UINT64_C(0xc4ceb9fe1a85ec53);
    h2 ^= h2 >> 33;

    h1 += h2;
    return h1;
}

uint64_t __attribute__((weak)) Hash_GetSeed()
{
#if defined(TEST_ENV)
    return 0x123456789abcdef0ull;
#else
    uint64_t seed = (uint64_t) (uintptr_t) &seed;
    seed         ^= (uint64_t) time(NULL);
    return seed;
#endif
}

void Hash_SipHash_Init_u64(uint64_t* k0, uint64_t* k1)
{
    uint64_t seed = Hash_GetSeed();

    *k0 = Hash_SplitMix64(seed);
    *k1 = Hash_SplitMix64(seed);
}


void Hash_SipHash_Init_u32(uint32_t* k0, uint32_t* k1)
{
    uint64_t seed = Hash_GetSeed();

    uint64_t x0 = Hash_SplitMix64(seed);
    uint64_t x1 = Hash_SplitMix64(seed);

    *k0 = (uint32_t) (x0 ^ (x0 >> 32));
    *k1 = (uint32_t) (x1 ^ (x1 >> 32));
}