#ifndef STD_HASHMAP_H__
#define STD_HASHMAP_H__

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

typedef struct HashMap_t
{
    size_t count;
    size_t capacity;

    HashMapEntry* head_entry;
    HashMapEntry* tail_entry;

    HashMapEntry** _buckets;
    size_t _seed[2];
    size_t _keyAlignment;
    size_t _valueAlignment;
    float _maxLoadFactor;
    float _minLoadFactor;
} HashMap_t;


#define HashMap(_keyT, _valueT, _size) HashMap_Create(_size, alignof(_keyT), alignof(_valueT));

// Son: Mom can we get a for each loop?
// Mom: No, we have a for each loop at home.
// For each loop at home:
#define HASHMAP_FOREACH(_e, _map)                                                               \
    for(HashMapEntry * (_e) = (_map) ? (_map)->head_entry : NULL; (_e); (_e) = (_e)->next_iter)

#define HASHMAP_FOREACH_REV(_e, _map)                                                           \
    for(HashMapEntry * (_e) = (_map) ? (_map)->tail_entry : NULL; (_e); (_e) = (_e)->prev_iter)

HashMap_t* HashMap_Create(size_t size, size_t keyAlignment, size_t valueAlignment);

void HashMap_Destroy(HashMap_t* map);

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

static inline int HashMapEntry_CompareKey(const HashMapEntry* entry, const void* key, size_t keySize)
{
    if(entry->_keySize != keySize)
    {
        return 1;
    }

    return memcmp(entry->data + entry->_keyOffset, key, keySize) != 0;
}

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