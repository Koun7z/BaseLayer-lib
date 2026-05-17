#ifndef BASE_STRING_H__
#define BASE_STRING_H__

#include "Base_HashMap.h"

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

typedef char* String_t;

typedef struct
{
    size_t length;
    size_t capacity;
    char data[];
} StringHeader_t;

#define String(str) String_Create(str, sizeof(str))

// clang-format off

#define ConstString(str)                     \
    ((const char*)(&(struct {                \
        StringHeader_t h;                    \
        char d[sizeof(str)];                 \
    }) {                                     \
        .h = {                               \
            .length        = sizeof(str),    \
            .capacity      = sizeof(str),    \
        },                                   \
        .d = str                             \
    }.d))

// clang-format on

/*
** Base string manipulation functions.
*/

static inline String_t String_Create(const char* str, size_t length)
{
    StringHeader_t* buff = (StringHeader_t*) malloc(sizeof(StringHeader_t) + length);
    if(buff)
    {
        buff->length   = length;
        buff->capacity = length;
        memcpy(buff->data, str, length);
        return buff->data;
    }
    return NULL;
}

static inline StringHeader_t* String_GetHeader(const char* str)
{
    return (StringHeader_t*) (str - offsetof(StringHeader_t, data));
}

/**
 * @brief Return the length of a string created by String_Create(), String() or ConstString()
 * @note  This length does not include the null terminator, even though it is included in every String_t instance.
 *
 * @param str
 * @return size_t
 */
static inline size_t String_Length(const char* str)
{
    return String_GetHeader(str)->length;
}

static inline char* String_Copy(const char* str)
{
    StringHeader_t* new_str = (StringHeader_t*) malloc(sizeof(StringHeader_t) + String_Length(str));
    if(new_str == NULL)
    {
        return NULL;
    }
    StringHeader_t* old_header = String_GetHeader(str);

    memcpy(new_str, old_header, sizeof(StringHeader_t) + old_header->length);
    return new_str->data;
}

static inline void String_Destroy(String_t* str)
{
    if(str == NULL)
    {
        return;
    }

    StringHeader_t* header = String_GetHeader(*str);

    free(header);
}

static inline void String_Clear(String_t str)
{
    StringHeader_t* header = String_GetHeader(str);

    header->length = 0;
}


// String map
#define StringMap(_T, _size) HashMap(char, _T, _size)

/*
** Hash map helpers for c string keys
*/

#define CStringMap_InsertFrom(_map, _key, _value, _size)             \
    HashMap_InsertFrom(_map, _key, strlen(_key) + 1, &_value, _size)

#define CStringMap_InsertPointer(_map, _key, _ptr) HashMap_InsertPointer(_map, _key, strlen(_key) + 1, _ptr)

#define CStringMap_Find(_map, _key) HashMap_Find(_map, _key, strlen(_key) + 1)

/*
** Hash map helpers for String_t keys
** Warning: Using those functions for c string not created by String_Create(), String() or ConstString()
** will cause undefined behavior since they expect a StringHeader_t before the string data.
*/

static inline HashMapEntry* FullStringMap_InsertFrom(HashMap_t* map, char* key, const void* value, size_t valueSize)
{
    return HashMap_InsertFrom(map, key, String_Length(key) + 1, &value, valueSize);
}

static inline HashMapEntry* FullStringMap_InsertPointer(HashMap_t* map, char* key, const void* value)
{
    return HashMap_InsertPointer(map, key, String_Length(key) + 1, value);
}

static inline HashMapEntry* FullStringMap_Find(const HashMap_t* map, char* key)
{
    return HashMap_Find(map, key, String_Length(key) + 1);
}

#endif /* BASE_STRING_H__ */