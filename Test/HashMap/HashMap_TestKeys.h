#ifndef HASHMAP_TESTKEYS_H__
#define HASHMAP_TESTKEYS_H__

#include "Base_Hash.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    void* data;
    size_t key_size;
    size_t count;
} key_table_t;


static inline key_table_t key_table_create(size_t count, size_t key_size, uint64_t seed)
{
    key_table_t t;
    t.count    = count;
    t.key_size = key_size;
    t.data     = malloc(count * key_size);

    if(!t.data)
    {
        t.count    = 0;
        t.key_size = 0;
        return t;
    }

    uint8_t* ptr = (uint8_t*) t.data;

    for(size_t i = 0; i < count; i++)
    {
        uint64_t value = Hash_SplitMix64(i ^ seed);

        size_t offset = i * key_size;

        if(key_size <= sizeof(uint64_t))
        {
            // Copy only needed bytes
            memcpy(ptr + offset, &value, key_size);
        }
        else
        {
            size_t written = 0;
            uint64_t v     = value;

            while(written < key_size)
            {
                size_t chunk = (key_size - written > sizeof(uint64_t)) ? sizeof(uint64_t) : (key_size - written);

                memcpy(ptr + offset + written, &v, chunk);

                v        = Hash_SplitMix64(v);
                written += chunk;
            }
        }
    }

    return t;
}

static inline void* key_table_get(const key_table_t* t, size_t index)
{
    if(index >= t->count)
    {
        return NULL;
    }

    return (uint8_t*) t->data + (index * t->key_size);
}

static inline void key_table_destroy(key_table_t* t)
{
    if(t && t->data)
    {
        free(t->data);
        t->data     = NULL;
        t->count    = 0;
        t->key_size = 0;
    }
}

#endif /* HASHMAP_TESTKEYS_H__ */