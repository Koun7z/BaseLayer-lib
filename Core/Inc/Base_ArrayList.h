#ifndef ArrayList_H__
#define ArrayList_H__

#include <stddef.h>
#include <stdalign.h>

typedef struct
{
    size_t count;
    size_t capacity;
    size_t _element_size;
    size_t _element_alignment;
    void* _data;
} ArrayList_t;

#define ArrayList(_T, _size) (ArrayList_Create(sizeof(_T), alignof(_T), _size))

ArrayList_t* ArrayList_Create(size_t elementSize, size_t element_alignment, size_t initialCapacity);

void ArrayList_Destroy(ArrayList_t* list);

#endif /* ArrayList_H__ */