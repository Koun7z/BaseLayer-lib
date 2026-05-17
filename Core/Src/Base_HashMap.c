#include "Base_HashMap.h"

#include "Base_Config.h"
#include "Base_Utils.h"
#include "Base_Hash.h"

#include <stdlib.h>
#include <assert.h>

#include <stdio.h>

static inline HashMapEntry* get_last_in_bucket(HashMapEntry* entry)
{
    assert(entry);

    while(entry->next_in_bucket)
    {
        entry = entry->next_in_bucket;
    }
    return entry;
}

static inline HashMapEntry* alloc_entry_aligned(size_t keySize,
                                                size_t valueSize,
                                                size_t keyAlignment,
                                                size_t valueAlignment)
{
    HashMapEntry* entry = NULL;
    size_t key_offset   = offsetof(HashMapEntry, data);
    size_t value_offset = offsetof(HashMapEntry, data) + keySize;
    if(keyAlignment <= alignof(max_align_t) && valueAlignment <= alignof(max_align_t))
    {
        value_offset = Utils_AlignUp(value_offset, alignof(max_align_t));

        entry = malloc(value_offset + valueSize);
    }
    else
    {
        size_t base_alignment = alignof(max_align_t);
        base_alignment        = Utils_Max(base_alignment, keyAlignment);
        base_alignment        = Utils_Max(base_alignment, valueAlignment);

        // TODO: Overflow checks
        size_t size_end = offsetof(HashMapEntry, data);
        size_end        = Utils_AlignUp(size_end, keyAlignment);
        key_offset      = size_end;
        size_end       += keySize;

        size_end     = Utils_AlignUp(size_end, valueAlignment);
        value_offset = size_end;
        size_end    += valueSize;

        size_t total_size = Utils_AlignUp(size_end, base_alignment);

        entry = aligned_alloc(base_alignment, total_size);
    }

    entry->next_in_bucket = NULL;
    entry->next_iter      = NULL;
    entry->prev_iter      = NULL;
    entry->_keySize       = keySize;
    entry->_valueSize     = valueSize;
    entry->_keyOffset     = key_offset - offsetof(HashMapEntry, data);
    entry->_valueOffset   = value_offset - offsetof(HashMapEntry, data);

    return entry;
}

// Move the entry to the end of the iterator linked list
static inline void update_tail_entry(HashMap_t* map, HashMapEntry* new_tail)
{
    HashMapEntry* tail = map->tail_entry;
    tail->next_iter    = new_tail;
    map->tail_entry    = new_tail;

    new_tail->prev_iter = tail;
    new_tail->next_iter = NULL;
}

// Replaces the data of entry, reallocates if necessary
// Assumes the keys are identical in value and data identical in type
static inline HashMapEntry* replace_entry_data(HashMap_t* map,
                                               HashMapEntry* old_entry,
                                               const void* new_data,
                                               size_t new_value_size)
{
    HashMapEntry* new_entry = NULL;

    // Just replace the value if theres enough space (prefer reallocation if the new value is much smaller)
    if(new_value_size <= old_entry->_valueSize
       && new_value_size * HASH_MAP_ENTRY_REDUCTION_REALLOC_THRESHOLD >= old_entry->_valueSize)
    {
        memcpy(old_entry->data + old_entry->_valueOffset, new_data, new_value_size);
        old_entry->_valueSize = new_value_size;

        new_entry = old_entry;

        if(old_entry->prev_iter)
        {
            old_entry->prev_iter->next_iter = new_entry->next_iter;
        }
    }
    else
    {
        new_entry = alloc_entry_aligned(old_entry->_keySize, new_value_size, map->_keyAlignment, map->_valueAlignment);
        if(!new_entry)
        {
            return NULL;
        }

        mempcpy(HashMapEntry_GetKey(new_entry), HashMapEntry_GetKey(old_entry), old_entry->_keySize);
        mempcpy(HashMapEntry_GetValue(new_entry), new_data, new_value_size);

        new_entry->next_in_bucket = old_entry->next_in_bucket;

        free(old_entry);
    }

    return new_entry;
}


size_t HashMap_GetHash(const void* data, size_t size, const size_t seed[2])
{
#if HASH_MAP_HASH_FUNCTION == 0
    return Hash_FNV1a(data, size);
#elif HASH_MAP_HASH_FUNCTION == 1
    return Hash_Murmur3(data, size, seed[0]);
#elif HASH_MAP_HASH_FUNCTION == 2
    return Hash_SipHash(data, size, seed[0], seed[1]);
#endif
}

HashMap_t* HashMap_Create(size_t size, size_t keyAlignment, size_t valueAlignment)
{
    assert((keyAlignment & (keyAlignment - 1)) == 0);
    assert((valueAlignment & (valueAlignment - 1)) == 0);

    size = Utils_NextPow2(Utils_Max(HASH_MAP_MIN_SIZE, size));

    HashMap_t* map = malloc(sizeof(HashMap_t));
    if(!map)
    {
        return NULL;
    }

    map->_buckets = calloc(size, sizeof(HashMapEntry*));
    if(!map->_buckets)
    {
        free(map);
        return NULL;
    }

    map->count           = 0;
    map->head_entry      = NULL;
    map->tail_entry      = NULL;
    map->capacity        = size;
    map->_maxLoadFactor  = HASH_MAP_DEFAULT_MAX_LOAD_FACTOR;
    map->_minLoadFactor  = HASH_MAP_DEFAULT_MIN_LOAD_FACTOR;
    map->_keyAlignment   = keyAlignment;
    map->_valueAlignment = valueAlignment;

#if HASH_MAP_HASH_FUNCTION != 0
    Hash_SipHash_Init(&map->_seed[0], &map->_seed[1]);
#endif

    return map;
}

void HashMap_Destroy(HashMap_t* map)
{
    assert(map);

    HashMapEntry* e = map->head_entry;
    while(e)
    {
        HashMapEntry* next = e->next_iter;
        free(e);
        e = next;
    }

    if(map->_buckets)
    {
        free(map->_buckets);
    }
    free(map);
}

HashMapEntry* HashMap_InsertFrom(HashMap_t* map, const void* key, size_t keySize, const void* value, size_t valueSize)
{
    const float load = HashMap_GetLoadFactor(map);
    if(load > map->_maxLoadFactor)
    {
        if(HashMap_Expand(map))
        {
            return NULL;
        }
    }

    const size_t hash       = HashMap_GetHash(key, keySize, map->_seed);
    const size_t bucket_idx = hash & (map->capacity - 1);

    if(map->head_entry == NULL)
    {
        HashMapEntry* entry = alloc_entry_aligned(keySize, valueSize, map->_keyAlignment, map->_valueAlignment);
        if(!entry)
        {
            return NULL;
        }

        mempcpy(entry->data + entry->_keyOffset, key, keySize);
        mempcpy(entry->data + entry->_valueOffset, value, valueSize);

        map->head_entry           = entry;
        map->tail_entry           = entry;
        map->_buckets[bucket_idx] = entry;
        map->count++;

        return entry;
    }

    HashMapEntry* bucket_head = map->_buckets[bucket_idx];
    if(bucket_head == NULL)
    {
        HashMapEntry* entry = alloc_entry_aligned(keySize, valueSize, map->_keyAlignment, map->_valueAlignment);
        if(!entry)
        {
            return NULL;
        }

        mempcpy(entry->data + entry->_keyOffset, key, keySize);
        mempcpy(entry->data + entry->_valueOffset, value, valueSize);

        update_tail_entry(map, entry);
        map->_buckets[bucket_idx] = entry;
        map->count++;

        return entry;
    }

    HashMapEntry* e         = bucket_head;
    HashMapEntry* new_entry = NULL;
    if(HashMapEntry_CompareKey(e, key, keySize) == 0)
    {
        new_entry = replace_entry_data(map, e, value, valueSize);
        if(new_entry)
        {
            update_tail_entry(map, new_entry);
        }
        return new_entry;
    }
    while(e->next_in_bucket)
    {
        if(HashMapEntry_CompareKey(e->next_in_bucket, key, keySize) == 0)
        {
            new_entry = replace_entry_data(map, e->next_in_bucket, value, valueSize);
            if(new_entry)
            {
                e->next_in_bucket = new_entry;
                update_tail_entry(map, new_entry);
            }
            return new_entry;
        }
        e = e->next_in_bucket;
    }

    new_entry = alloc_entry_aligned(keySize, valueSize, map->_keyAlignment, map->_valueAlignment);
    if(!new_entry)
    {
        return NULL;
    }

    mempcpy(new_entry->data + new_entry->_keyOffset, key, keySize);
    mempcpy(new_entry->data + new_entry->_valueOffset, value, valueSize);

    e->next_in_bucket = new_entry;
    update_tail_entry(map, new_entry);
    map->count++;

    return new_entry;
}

int HashMap_Expand(HashMap_t* map)
{
    const size_t new_size = map->capacity * 2;

    return HashMap_Resize(map, new_size);
}

int HashMap_Resize(HashMap_t* map, size_t newSize)
{
    newSize = Utils_NextPow2(newSize);

    HashMapEntry** new_buckets = calloc(newSize, sizeof(HashMapEntry*));
    if(!new_buckets)
    {
        return -1;
    }
    free(map->_buckets);

    map->_buckets = new_buckets;
    map->capacity = newSize;

    HashMapEntry* e = map->head_entry;
    while(e)
    {
        const size_t hash       = HashMap_GetHash(HashMapEntry_GetKey(e), e->_keySize, map->_seed);
        const size_t bucket_idx = hash & (newSize - 1);

        e->next_in_bucket = NULL;
        if(new_buckets[bucket_idx] == NULL)
        {
            new_buckets[bucket_idx] = e;
        }
        else
        {
            get_last_in_bucket(new_buckets[bucket_idx])->next_in_bucket = e;
        }

        e = e->next_iter;
    }

    return 0;
}

HashMapEntry* HashMap_Find(const HashMap_t* map, const void* key, size_t keySize)
{
    const size_t hash       = HashMap_GetHash(key, keySize, map->_seed);
    const size_t bucket_idx = hash & (map->capacity - 1);

    HashMapEntry* e = map->_buckets[bucket_idx];
    while(e)
    {
        if(HashMapEntry_CompareKey(e, key, keySize) == 0)
        {
            return e;
        }
        e = e->next_in_bucket;
    }

    return NULL;
}

static inline void remove_from_iter(HashMap_t* map, HashMapEntry* e)
{
    if(e->prev_iter)
    {
        e->prev_iter->next_iter = e->next_iter;
    }
    else
    {
        map->head_entry = e->next_iter;
    }

    if(e->next_iter)
    {
        e->next_iter->prev_iter = e->prev_iter;
    }
    else
    {
        map->tail_entry = e->prev_iter;
    }
}

void HashMap_RemoveElement(HashMap_t* map, const void* key, size_t keySize)
{
    if(map->capacity > HASH_MAP_MIN_SIZE && HashMap_GetLoadFactor(map) < map->_minLoadFactor)
    {
        HashMap_Resize(map, map->capacity / 2);
    }

    const size_t hash       = HashMap_GetHash(key, keySize, map->_seed);
    const size_t bucket_idx = hash & (map->capacity - 1);

    HashMapEntry* e = map->_buckets[bucket_idx];
    if(e == NULL)
    {
        return;
    }

    if(HashMapEntry_CompareKey(e, key, keySize) == 0)
    {
        map->_buckets[bucket_idx] = e->next_in_bucket;
        remove_from_iter(map, e);

        free(e);
        map->count--;
        return;
    }
    while(e->next_in_bucket)
    {
        if(HashMapEntry_CompareKey(e->next_in_bucket, key, keySize) == 0)
        {
            HashMapEntry* to_remove = e->next_in_bucket;
            e->next_in_bucket       = to_remove->next_in_bucket;

            remove_from_iter(map, to_remove);

            free(to_remove);
            map->count--;
            return;
        }

        e = e->next_in_bucket;
    }
}