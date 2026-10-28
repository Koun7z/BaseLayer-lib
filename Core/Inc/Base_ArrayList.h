#ifndef ArrayList_H__
#define ArrayList_H__

#include "Base_Allocator.h"
#include "Base_Macros.h"
#include "Base_Config.h"

#include <stddef.h>
#include <stdalign.h>
#include <assert.h>

typedef struct
{
    size_t count;
    size_t capacity;
    size_t _element_size;
    const Allocator_t* _allocator;
    void* _data;
} ArrayList_t;

#define ARRAYLIST_DEFAULT_CONSTRUCTOR(_T)                                                          \
    ArrayList_Create(sizeof(_T), ARRAY_LIST_DEFAULT_INITIAL_CAPACITY, Alloc_GetDefaultAllocator())
#define ARRAYLIST_INITIAL_SIZE(_T, _n)         ArrayList_Create(sizeof(_T), (_n), Alloc_GetDefaultAllocator())
#define ARRAYLIST_CUSTOM_ALLOC(_T, _n, _alloc) ArrayList_Create(sizeof(_T), (_n), (_alloc))

#define ArrayList(...)                                                                   \
    SELECT_PARAM_SET_1_TO_3(__VA_ARGS__, ARRAYLIST_CUSTOM_ALLOC, ARRAYLIST_INITIAL_SIZE, \
                            ARRAYLIST_DEFAULT_CONSTRUCTOR)(__VA_ARGS__)

/**
 * @brief Gets the VALUE of the element at the specified index in the array list.
 *
 */
#define ArrayList_GetAs(_T, list, idx) (*(_T*) ArrayList_Get(list, idx))

#define ArrayList_CArray(_T, list) ((_T*) ((list)->_data))

ArrayList_t* ArrayList_Create(size_t elementSize, size_t initialCapacity, const Allocator_t* alloc);

int ArrayList_Init(ArrayList_t* list, size_t elementSize, size_t initialCapacity, const Allocator_t* alloc);

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