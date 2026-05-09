#ifndef BASE_STRING_H__
#define BASE_STRING_H__

#include "Base_ArrayList.h"

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

#define String_GetHeader(str) ((StringHeader_t*) (str - sizeof(StringHeader_t)))
#define String_Length(str)    (String_GetHeader(str)->length)

// clang-format off

#define ConstString(str)                                        \
    ((const char*)(&(struct {                                   \
        StringHeader_t h;                                       \
        char d[sizeof(str)];                                    \
    }) {                                                        \
        .h = {                                                  \
            .length        = sizeof(str),                       \
            .capacity         = sizeof(str),                    \
        },                                                      \
        .d = str                                                \
    }.d))

// clang-format on

#define StringMap(_T, _size) HashMap(char, _T, _size)
#define StringMap_Insert(_map, _key, _value, _size)                                      \
    HashMap_InsertElement(_map, _key, sizeof(char) * (strlen(_key) + 1), &_value, _size)

#define StringMap_InsertArray(_valueT, _map, _key, _value, _count)                                         \
    HashMap_InsertElement(_map, _key, sizeof(char) * (strlen(_key) + 1), _value, sizeof(_valueT) * _count)

#define StringMap_Find(_map, _key) HashMap_FindElement(_map, _key, strlen(_key) + 1)

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
    ArrayList_ClearList(str);
}

#endif /* BASE_STRING_H__ */