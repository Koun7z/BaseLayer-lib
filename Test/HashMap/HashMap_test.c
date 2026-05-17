#include "TestRegistry.h"

#include "Base_HashMap.h"
#include "HashMap_TestKeys.h"

#include <stdio.h>
#include <stdbool.h>

extern key_table_t test_keys;

START_TEST(Insertion_test)
{
    typedef struct
    {
        int a;
        float b;
    } test_struct_t;

    HashMap_t* map = HashMap(char, test_struct_t, 16);
    ck_assert(map != NULL);

    char* keys[]           = {"never", "gonna", "give", "you", "up"};
    test_struct_t values[] = {
        {.a = 1, .b = 1.0f},
        {.a = 2, .b = 2.0f},
        {.a = 3, .b = 3.0f},
        {.a = 4, .b = 4.0f},
        {.a = 5, .b = 5.0f},
    };

    for(size_t i = 0; i < 5; i++)
    {
        HashMap_InsertFrom(map, keys[i], strlen(keys[i]) + 1, &values[i], sizeof(test_struct_t));
    }
    ck_assert(map->count == 5);

    for(int i = sizeof(keys) / sizeof(keys[0]) - 1; i >= 0; i--)
    {
        HashMapEntry* e = HashMap_Find(map, keys[i], strlen(keys[i]) + 1);
        ck_assert(e != NULL);

        test_struct_t* v = HashMapEntry_GetValueAs(test_struct_t, e);
        ck_assert(v->a == values[i].a);
        ck_assert(v->b == values[i].b);
    }

    HashMap_Destroy(map);
}

START_TEST(Resize_test)
{
    HashMap_t* map = HashMap(char, int, 16);
    ck_assert(map != NULL);

    for(size_t i = 0; i < test_keys.count; i++)
    {
        void* key = key_table_get(&test_keys, i);
        HashMap_InsertFrom(map, key, test_keys.key_size, &i, sizeof(size_t));
    }
    ck_assert(map->count == test_keys.count);

    for(size_t i = 0; i < test_keys.count; i++)
    {
        void* key       = key_table_get(&test_keys, i);
        HashMapEntry* e = HashMap_Find(map, key, test_keys.key_size);
        ck_assert(e != NULL);

        size_t v = *HashMapEntry_GetValueAs(size_t, e);
        ck_assert(v == i);
    }

    HashMap_Destroy(map);
}


START_TEST(Iteration_test)
{
    HashMap_t* map = HashMap(char, int, 16);
    ck_assert(map != NULL);

    for(size_t i = 0; i < test_keys.count; i++)
    {
        void* key = key_table_get(&test_keys, i);
        HashMap_InsertFrom(map, key, test_keys.key_size, &i, sizeof(size_t));
    }

    // Check forward iteration
    size_t count = 0;
    HASHMAP_FOREACH(e, map)
    {
        size_t v = *HashMapEntry_GetValueAs(size_t, e);
        ck_assert(v == count);
        count++;
    }

    // Check backward iteration
    count = map->count - 1;
    HASHMAP_FOREACH_REV(e, map)
    {
        size_t v = *HashMapEntry_GetValueAs(size_t, e);
        ck_assert(v == count);
        count--;
    }

    HashMap_Destroy(map);
}

static inline bool is_prime(size_t n)
{
    if(n <= 1)
    {
        return false;
    }
    if(n <= 3)
    {
        return true;
    }
    if(n % 2 == 0 || n % 3 == 0)
    {
        return false;
    }

    // Check divisors up to sqrt(n), skipping multiples of 2 and 3
    for(size_t i = 5; i * i <= n; i += 6)
    {
        if(n % i == 0 || n % (i + 2) == 0)
        {
            return false;
        }
    }
    return true;
}

START_TEST(Removal_test)
{
    HashMap_t* map = HashMap(char, int, 16);
    ck_assert(map != NULL);

    for(size_t i = 0; i < test_keys.count; i++)
    {
        void* key = key_table_get(&test_keys, i);
        HashMap_InsertFrom(map, key, test_keys.key_size, &i, sizeof(size_t));
    }

    // Remove elements at every prime index
    size_t rm_cnt = 0;
    for(size_t i = 0; i < test_keys.count; i++)
    {
        if(is_prime(i))
        {
            void* key = key_table_get(&test_keys, i);
            HashMap_RemoveElement(map, key, test_keys.key_size);
            rm_cnt++;
        }
    }

    ck_assert(map->count == test_keys.count - rm_cnt);

    // Check remaining elements
    HASHMAP_FOREACH(e, map)
    {
        size_t v = *HashMapEntry_GetValueAs(size_t, e);
        ck_assert(!is_prime(v));
    }

    HASHMAP_FOREACH_REV(e, map)
    {
        size_t v = *HashMapEntry_GetValueAs(size_t, e);
        ck_assert(!is_prime(v));
    }

    // Manual iteration through buckets
    // Check if all pointers got properly removed
    for(size_t i = 0; i < map->capacity; i++)
    {
        for(HashMapEntry* e = map->_buckets[i]; e; e = e->next_in_bucket)
        {
            size_t v = *HashMapEntry_GetValueAs(size_t, e);
            ck_assert(!is_prime(v));
        }
    }

    HashMap_Destroy(map);
}

START_TEST(big_align_test)
{
    typedef struct __attribute__((aligned(64)))
    {
        uint64_t data[8];
    } align64_t;

    HashMap_t* map64 = HashMap(align64_t, align64_t, 16);
    ck_assert(map64 != NULL);

    for(size_t i = 0; i < test_keys.count; i++)
    {
        void* key       = key_table_get(&test_keys, i);
        align64_t value = {
            .data = {i, i, i, i, i, i, i, i}
        };
        char big_key[64];
        for(size_t j = 0; j < 64; j++)
        {
            big_key[j] = ((char*) key)[j % test_keys.key_size];
        }
        HashMapEntry* e = HashMap_InsertFrom(map64, big_key, 64, &value, sizeof(align64_t));

        void* k = HashMapEntry_GetKey(e);
        void* v = HashMapEntry_GetValue(e);

        ck_assert((uint64_t) k % 64 == 0);
        ck_assert((uint64_t) v % 64 == 0);
    }

    HashMap_t* map512 = HashMap_Create(16, 512, 512);
    ck_assert(map512 != NULL);

    for(size_t i = 0; i < test_keys.count; i++)
    {
        void* key       = key_table_get(&test_keys, i);
        align64_t value = {
            .data = {i, i, i, i, i, i, i, i}
        };

        HashMapEntry* e = HashMap_InsertFrom(map512, key, test_keys.key_size, &value, sizeof(align64_t));

        void* k = HashMapEntry_GetKey(e);
        void* v = HashMapEntry_GetValue(e);

        ck_assert((uint64_t) k % 512 == 0);
        ck_assert((uint64_t) v % 512 == 0);
    }

    HashMap_Destroy(map64);
    HashMap_Destroy(map512);
}

__attribute__((constructor)) void register_hashmap_suite(void)
{
    TR_AddTest("HashMap", "Core", Insertion_test);
    TR_AddTest("HashMap", "Core", Resize_test);
    TR_AddTest("HashMap", "Core", Iteration_test);
    TR_AddTest("HashMap", "Core", Removal_test);
    TR_AddTest("HashMap", "Core", big_align_test);
}
