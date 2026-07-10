#ifndef ArrayList_H__
#define ArrayList_H__

#include <stddef.h>
#include <stdalign.h>
#include <assert.h>

typedef struct
{
    size_t count;
    size_t capacity;
    size_t _element_size;
    size_t _element_alignment;
    void* _data;
} ArrayList_t;

#define ArrayList(_T, _size) (ArrayList_Create(sizeof(_T), alignof(_T), _size))

/**
 * @brief Gets the VALUE of the element at the specified index in the array list.
 *
 */
#define ArrayList_GetAs(_T, list, idx) (*(_T*) ArrayList_Get(list, idx))

#define ArrayList_CArray(_T, list) ((_T*) ((list)->_data))

ArrayList_t* ArrayList_Create(size_t elementSize, size_t element_alignment, size_t initialCapacity);

void ArrayList_Destroy(ArrayList_t* list);

int ArrayList_Append(ArrayList_t* list, const void* element);
int ArrayList_Insert(ArrayList_t* list, size_t idx, const void* element);

int ArrayList_AppendRange(ArrayList_t* list, const void* elements, size_t count);
int ArrayList_InsertRange(ArrayList_t* list, size_t idx, const void* elements, size_t count);

/**
 * @brief Realloc the underlying array to have the specified capacity.
 *        New capacity can be smaller, it will delete your data if not carefull.
 *
 * @param list
 * @param n
 * @return int
 */
int ArrayList_Realloc(ArrayList_t* list, size_t capacity);

/**
 * @brief Expands the underlying array so that at least n more elements can be inserted without reallocating.
 *
 * @param list
 * @param n
 * @return int
 */
int ArrayList_Reserve(ArrayList_t* list, size_t n);

static inline void* ArrayList_Get(ArrayList_t* list, size_t idx)
{
    assert(list != NULL);
    assert(idx < list->count);

    return ((char*) list->_data + (idx * list->_element_size));
}

void ArrayList_Remove(ArrayList_t* list, size_t idx);
void ArrayList_RemoveRange(ArrayList_t* list, size_t idx, size_t count);

void ArrayList_Pop(ArrayList_t* list);
void ArrayList_PopRange(ArrayList_t* list, size_t n);

void ArrayList_Clear(ArrayList_t* list);

int ArrayList_Trim(ArrayList_t* list);

#endif /* ArrayList_H__ */