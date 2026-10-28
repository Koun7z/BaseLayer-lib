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

static inline HashMapEntry* alloc_entry_aligned(size_t keySize, size_t valueSize, const KV_Allocator_t* alloc)
{
    HashMapEntry* entry = NULL;
    size_t key_offset   = offsetof(HashMapEntry, data);
    size_t value_offset = offsetof(HashMapEntry, data) + keySize;
    if(alloc->key_alignement <= alignof(max_align_t) && alloc->value_alignement <= alignof(max_align_t))
    {
        value_offset = Utils_AlignUp(value_offset, alignof(max_align_t));

        entry = alloc->alloc(value_offset + valueSize);
    }
    else
    {
        const size_t max_align = Utils_Max(alloc->key_alignement, alloc->value_alignement);

        // TODO: Overflow checks
        size_t size_end = offsetof(HashMapEntry, data);
        size_end        = Utils_AlignUp(size_end, alloc->key_alignement);
        key_offset      = size_end;
        size_end       += keySize;

        size_end     = Utils_AlignUp(size_end, alloc->value_alignement);
        value_offset = size_end;
        size_end    += valueSize;

        // Allocate n * alignment bytes
        const size_t total_size = Utils_AlignUp(size_end, max_align);

        // Assume that the allocator will return a pointer aligned to alloc->alignement
        entry = alloc->alloc(total_size);
        assert(((uintptr_t) entry % max_align) == 0);
    }

    if(entry == NULL)
    {
        return NULL;
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
        new_entry = alloc_entry_aligned(old_entry->_keySize, new_value_size, map->_kv_allocator);
        if(!new_entry)
        {
            return NULL;
        }

        memcpy(HashMapEntry_GetKey(new_entry), HashMapEntry_GetKey(old_entry), old_entry->_keySize);
        memcpy(HashMapEntry_GetValue(new_entry), new_data, new_value_size);

        new_entry->next_in_bucket = old_entry->next_in_bucket;

        map->_bucket_allocator->free(old_entry);
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

static int default_key_compare(const void* key1, size_t size1, const void* key2, size_t size2)
{
    if(size1 != size2)
    {
        return 1;
    }

    return memcmp(key1, key2, size1) != 0;
}

int HashMap_Init(HashMap_t* map, size_t size, const KV_Allocator_t* kv_alloc, const Allocator_t* bucket_alloc)
{
    assert(map);
    assert(bucket_alloc);
    assert(kv_alloc);

    size = Utils_NextPow2(Utils_Max(HASH_MAP_MIN_SIZE, size));

    map->_buckets = calloc(size, sizeof(HashMapEntry*));
    if(!map->_buckets)
    {
        return -1;
    }

    map->count             = 0;
    map->head_entry        = NULL;
    map->tail_entry        = NULL;
    map->capacity          = size;
    map->_max_load_actor   = HASH_MAP_DEFAULT_MAX_LOAD_FACTOR;
    map->_min_load_factor  = HASH_MAP_DEFAULT_MIN_LOAD_FACTOR;
    map->_bucket_allocator = bucket_alloc;
    map->_kv_allocator     = kv_alloc;
    map->_key_cmp_func     = default_key_compare;

#if HASH_MAP_HASH_FUNCTION != 0
    Hash_SipHash_Init(&map->_seed[0], &map->_seed[1]);
#endif

    return 0;
}

HashMap_t* HashMap_Create(size_t size, const KV_Allocator_t* kv_alloc, const Allocator_t* bucket_alloc)
{
    assert(bucket_alloc);

    HashMap_t* map = bucket_alloc->alloc(sizeof(HashMap_t));
    if(!map)
    {
        return NULL;
    }

    if(HashMap_Init(map, size, kv_alloc, bucket_alloc) != 0)
    {
        bucket_alloc->free(map);
        return NULL;
    }

    return map;
}

void HashMap_Destroy(HashMap_t* map)
{
    assert(map);
    assert(map->_bucket_allocator && map->_bucket_allocator->free);

    HashMapEntry* e = map->head_entry;
    while(e)
    {
        HashMapEntry* next = e->next_iter;
        map->_bucket_allocator->free(e);
        e = next;
    }

    if(map->_buckets)
    {
        map->_bucket_allocator->free(map->_buckets);
    }
    map->_bucket_allocator->free(map);
}

HashMapEntry* HashMap_InsertFrom(HashMap_t* map, const void* key, size_t keySize, const void* value, size_t valueSize)
{
    const float load = HashMap_GetLoadFactor(map);
    if(load > map->_max_load_actor)
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
        HashMapEntry* entry = alloc_entry_aligned(keySize, valueSize, map->_kv_allocator);
        if(!entry)
        {
            return NULL;
        }

        memcpy(entry->data + entry->_keyOffset, key, keySize);
        memcpy(entry->data + entry->_valueOffset, value, valueSize);

        map->head_entry           = entry;
        map->tail_entry           = entry;
        map->_buckets[bucket_idx] = entry;
        map->count++;

        return entry;
    }

    HashMapEntry* bucket_head = map->_buckets[bucket_idx];
    if(bucket_head == NULL)
    {
        HashMapEntry* entry = alloc_entry_aligned(keySize, valueSize, map->_kv_allocator);
        if(!entry)
        {
            return NULL;
        }

        memcpy(entry->data + entry->_keyOffset, key, keySize);
        memcpy(entry->data + entry->_valueOffset, value, valueSize);

        update_tail_entry(map, entry);
        map->_buckets[bucket_idx] = entry;
        map->count++;

        return entry;
    }

    HashMapEntry* e         = bucket_head;
    HashMapEntry* new_entry = NULL;
    if(map->_key_cmp_func(HashMapEntry_GetKey(e), e->_keySize, key, keySize) == 0)
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
        if(map->_key_cmp_func(HashMapEntry_GetKey(e->next_in_bucket), e->next_in_bucket->_keySize, key, keySize) == 0)
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

    new_entry = alloc_entry_aligned(keySize, valueSize, map->_kv_allocator);
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
        if(map->_key_cmp_func(HashMapEntry_GetKey(e), e->_keySize, key, keySize) == 0)
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
    if(map->capacity > HASH_MAP_MIN_SIZE && HashMap_GetLoadFactor(map) < map->_min_load_factor)
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

    if(map->_key_cmp_func(HashMapEntry_GetKey(e), e->_keySize, key, keySize) == 0)
    {
        map->_buckets[bucket_idx] = e->next_in_bucket;
        remove_from_iter(map, e);

        free(e);
        map->count--;
        return;
    }
    while(e->next_in_bucket)
    {
        if(map->_key_cmp_func(HashMapEntry_GetKey(e->next_in_bucket), e->next_in_bucket->_keySize, key, keySize) == 0)
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