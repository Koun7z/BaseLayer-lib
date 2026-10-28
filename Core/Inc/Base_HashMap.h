#ifndef STD_HASHMAP_H__
#define STD_HASHMAP_H__

#include "Base_Allocator.h"
#include "Base_Macros.h"
#include "Base_Config.h"

#include <stddef.h>
#include <stdalign.h>
#include <string.h>

/**
 * @brief A hash map implementation that supports arbitrary key and value types, custom alignment requirements and insertion order iteration.
 *
 * @details Implemented as a separate chaining hash map with a linked list for each bucket,
            and an additional two-way linked list to maintain insertion order.
 *
 */
typedef struct HashMap_t HashMap_t;

typedef struct HashMapEntry HashMapEntry;
typedef struct HashMapEntry
{
    // In bucket one-way linked list
    HashMapEntry* next_in_bucket;

    // Insertion order two-way iteration linked list
    HashMapEntry* next_iter;
    HashMapEntry* prev_iter;

    // Key and value metadata
    size_t _keySize;
    size_t _valueSize;
    size_t _keyOffset;
    size_t _valueOffset;
    alignas(max_align_t) char data[];
} HashMapEntry;


typedef int (*KeyCmpFunc_t)(const void* key1, size_t size1, const void* key2, size_t size2);

typedef struct HashMap_t
{
    size_t count;
    size_t capacity;

    HashMapEntry* head_entry;
    HashMapEntry* tail_entry;

    HashMapEntry** _buckets;
    size_t _seed[2];
    const Allocator_t* _bucket_allocator;
    const KV_Allocator_t* _kv_allocator;
    KeyCmpFunc_t _key_cmp_func;
    float _max_load_actor;
    float _min_load_factor;
} HashMap_t;

#define HASHMAP_DEFAULT_ALL_IMPL()                                                                \
    HashMap_Create(HASH_MAP_MIN_SIZE, Alloc_GetDefaultKVAllocator(), Alloc_GetDefaultAllocator())
#define HASHMAP_DEFAULT_ALLOC_IMPL(_n) HashMap_Create((_n), Alloc_GetDefaultKVAllocator(), Alloc_GetDefaultAllocator())
#define HASHMAP_DEFAULT_BUCKET_IMPL(_n, _kv_alloc)        HashMap_Create((_n), (_kv_alloc), Alloc_GetDefaultAllocator())
#define HASHMAP_CUSTOM_IMPL(_n, _kv_alloc, _bucket_alloc) HashMap_Create((_n), (_kv_alloc), (_bucket_alloc))

/**
 * @brief Usage:
 *        HashMap()         - Allocates a new hash map with default size (16) and default allocator (using malloc). (GNUC only)
 *        HashMap(size_t n) - Allocates a new hash map with 'n' empty buckets and default allocator (using malloc).
 *                            Equivalent to HashMap_Create(n, Alloc_GetDefaultKVAllocator(), Alloc_GetDefaultAllocator()).
 *
 *        HashMap(size_t n, KV_Allocator_t kv_alloc, Allocator_t bucket_alloc) -
 *          Allocates a new hash map with n empty buckets default buckets allocator and custom kv pair allocator.
 *        HashMap(size_t n, KV_Allocator_t kv_alloc, Allocator_t bucket_alloc) -
 *          Allocates a new hash map with n empty buckets and the specified allocators for allocating buckets and key/value pairs.
 *
 * @note  See Base_Allocator.h for more information on how to implement custom allocators.
 */
#ifdef __GNUC__
#  define HashMap(...)                                                                           \
      SELECT_PARAM_SET_0_TO_3(, ##__VA_ARGS__, HASHMAP_CUSTOM_IMPL, HASHMAP_DEFAULT_BUCKET_IMPL, \
                              HASHMAP_DEFAULT_ALLOC_IMPL, HASHMAP_DEFAULT_ALL_IMPL)(__VA_ARGS__)
#else
#  define HashMap(...)                                                                       \
      SELECT_PARAM_SET_1_TO_3(__VA_ARGS__, HASHMAP_CUSTOM_IMPL, HASHMAP_DEFAULT_BUCKET_IMPL, \
                              HASHMAP_DEFAULT_ALLOC_IMPL)(__VA_ARGS__)
#endif

// Son: Mom can we get a for each loop?
// Mom: No, we have a for each loop at home.
// For each loop at home:
#define HASHMAP_FOREACH(_e, _map)                                                               \
    for(HashMapEntry * (_e) = (_map) ? (_map)->head_entry : NULL; (_e); (_e) = (_e)->next_iter)

#define HASHMAP_FOREACH_REV(_e, _map)                                                           \
    for(HashMapEntry * (_e) = (_map) ? (_map)->tail_entry : NULL; (_e); (_e) = (_e)->prev_iter)

HashMap_t* HashMap_Create(size_t size, const KV_Allocator_t* kv_alloc, const Allocator_t* bucket_alloc);

int HashMap_Init(HashMap_t* map, size_t size, const KV_Allocator_t* kv_alloc, const Allocator_t* bucket_alloc);

void HashMap_Destroy(HashMap_t* map);

void HashMap_SetKeyCompareFunc(HashMap_t* map, KeyCmpFunc_t cmp_func);

HashMapEntry* HashMap_InsertFrom(HashMap_t* map, const void* key, size_t keySize, const void* value, size_t valueSize);

static inline HashMapEntry* HashMap_InsertPointer(HashMap_t* map, const void* key, size_t keySize, const void* value)
{
    return HashMap_InsertFrom(map, key, keySize, &value, sizeof(void*));
}

HashMapEntry* HashMap_Find(const HashMap_t* map, const void* key, size_t keySize);

void HashMap_RemoveElement(HashMap_t* map, const void* key, size_t keySize);

size_t HashMap_GetHash(const void* data, size_t size, const size_t seed[2]);

int HashMap_Expand(HashMap_t* map);

int HashMap_Resize(HashMap_t* map, size_t newSize);

static inline float HashMap_GetLoadFactor(const HashMap_t* map)
{
    return (float) map->count / (float) map->capacity;
}

static inline void* HashMapEntry_GetKey(const HashMapEntry* entry)
{
    return (void*) (entry->data + entry->_keyOffset);
}

static inline void* HashMapEntry_GetValue(const HashMapEntry* entry)
{
    return (void*) (entry->data + entry->_valueOffset);
}

#define HashMapEntry_GetValueAs(_valueT, _entry) ((_valueT*) HashMapEntry_GetValue(_entry))

#endif  // STD_HASHMAP_H__