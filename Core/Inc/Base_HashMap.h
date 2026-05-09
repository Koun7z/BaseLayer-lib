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
    size_t seed[2];
    HashMapEntry** buckets;

    HashMapEntry* head_entry;
    HashMapEntry* tail_entry;

    size_t _keyAlignment;
    size_t _valueAlignment;
    float _maxLoadFactor;
    float _minLoadFactor;
} HashMap_t;


#define HashMap(_keyT, _valueT, _size) HashMap_Create(_size, alignof(_keyT), alignof(_valueT));

#define HashMap_Insert(_keyT, _valueT, _map, _key, _value)                     \
    HashMap_InsertElement(_map, _key, sizeof(_keyT), &_value, sizeof(_valueT))
#define HashMap_InsertArray(_keyT, _valueT, _map, _key, _value, _count)                 \
    HashMap_InsertElement(_map, &_key, sizeof(_keyT), _value, sizeof(_valueT) * _count)

#define HashMap_Find(_keyT, _valueT, _map, _key) ((valueT*) HashMap_FindElement(_map, &_key, sizeof(_keyT)))


HashMap_t* HashMap_Create(size_t size, size_t keyAlignment, size_t valueAlignment);

HashMapEntry* HashMap_InsertElement(HashMap_t* map,
                                    const void* key,
                                    size_t keySize,
                                    const void* value,
                                    size_t valueSize);

HashMapEntry* HashMap_FindElement(const HashMap_t* map, const void* key, size_t keySize);

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

#endif  // STD_HASHMAP_H__