#ifndef BASE_HASH_H__
#define BASE_HASH_H__

#include <stddef.h>
#include <limits.h>
#include <stdint.h>

typedef enum
{
    HASH_FUNCTION_FNV1A   = 0,
    HASH_FUNCTION_MURMUR3 = 1,
    HASH_FUNCTION_SIPHASH = 2
} HashFunction_t;


static inline size_t Hash_SplitMix64(uint64_t x)
{
    x += UINT64_C(0x9e3779b97f4a7c15);
    x  = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x  = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x  = x ^ (x >> 31);
    return x;
}

static inline uint32_t Hash_FNV1a_u32(const void* data, size_t size)
{
    uint32_t hash        = UINT32_C(0x811c9dc5);
    const uint32_t prime = UINT32_C(0x1000193);

    const unsigned char* p = data;
    for(size_t i = 0; i < size; i++)
    {
        hash ^= (uint32_t) p[i];
        hash *= (prime);
    }

    return hash;
}

static inline uint64_t Hash_FNV1a_u64(const void* data, size_t size)
{
    uint64_t hash        = UINT64_C(0xcbf29ce484222325);
    const uint64_t prime = UINT64_C(0x100000001b3);

    const unsigned char* p = data;
    for(size_t i = 0; i < size; i++)
    {
        hash ^= (uint64_t) p[i];
        hash *= (prime);
    }

    return hash;
}

static inline size_t Hash_FNV1a(const void* data, size_t size)
{
#if SIZE_MAX == UINT64_MAX
    return Hash_FNV1a_u64(data, size);
#elif SIZE_MAX == UINT32_MAX
    return Hash_FNV1a_u32(data, size);
#else
#  error "Unsupported size_t size"
#endif
}

uint32_t Hash_Murmur3_u32(const void* key, size_t len, uint32_t seed);

uint64_t Hash_Murmur3_u64(const void* key, size_t len, uint64_t seed);

static inline size_t Hash_Murmur3(const void* key, size_t len, size_t seed)
{
#if SIZE_MAX == UINT64_MAX
    return Hash_Murmur3_u64(key, len, seed);
#elif SIZE_MAX == UINT32_MAX
    return Hash_Murmur3_u32(key, len, seed);
#else
#  error "Unsupported size_t size"
#endif
}

uint32_t Hash_SipHash_u32(const void* key, size_t len, uint32_t k0, uint32_t k1);
uint64_t Hash_SipHash_u64(const void* key, size_t len, uint64_t k0, uint64_t k1);

static inline size_t Hash_SipHash(const void* key, size_t len, size_t k0, size_t k1)
{
#if SIZE_MAX == UINT64_MAX
    return Hash_SipHash_u64(key, len, k0, k1);
#elif SIZE_MAX == UINT32_MAX
    return Hash_SipHash_u32(key, len, k0, k1);
#else
#  error "Unsupported size_t size"
#endif
}

uint64_t Hash_GetSeed();

void Hash_SipHash_Init_u32(uint32_t* k0, uint32_t* k1);
void Hash_SipHash_Init_u64(uint64_t* k0, uint64_t* k1);

static inline void Hash_SipHash_Init(size_t* k0, size_t* k1)
{
#if SIZE_MAX == UINT64_MAX
    Hash_SipHash_Init_u64((uint64_t*) k0, (uint64_t*) k1);
#elif SIZE_MAX == UINT32_MAX
    Hash_SipHash_Init_u32((uint32_t*) k0, (uint32 _t*) k1);
#else
#  error "Unsupported size_t size"
#endif
}


#endif  // BASE_HASH_H__