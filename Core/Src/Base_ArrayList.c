#include "Base_ArrayList.h"

#include "Base_Utils.h"
#include "Base_Config.h"

#include <stdlib.h>
#include <string.h>

static inline void* array_alloc(size_t size, size_t alignment)
{
    if(alignment <= alignof(max_align_t))
    {
        return malloc(size);
    }

    return aligned_alloc(alignment, Utils_AlignUp(size, alignment));
}

static inline void* array_realloc(void* ptr, size_t old_size, size_t new_size, size_t alignment)
{
    if(alignment <= alignof(max_align_t))
    {
        return realloc(ptr, new_size);
    }

    // No aligned_realloc :(
    void* new_ptr = aligned_alloc(alignment, Utils_AlignUp(new_size, alignment));
    if(new_ptr == NULL)
    {
        return NULL;
    }

    memcpy(new_ptr, ptr, Utils_Min(old_size, new_size));
    free(ptr);

    return new_ptr;
}

ArrayList_t* ArrayList_Create(size_t elementSize, size_t element_alignment, size_t initialCapacity)
{
    assert((element_alignment & (element_alignment - 1)) == 0);

    ArrayList_t* list = (ArrayList_t*) malloc(sizeof(ArrayList_t));
    if(!list)
    {
        return NULL;
    }

    if(initialCapacity == 0)
    {
        initialCapacity = ARRAY_LIST_MIN_EXPAND_SIZE;
    }

    list->count              = 0;
    list->capacity           = initialCapacity;
    list->_element_size      = elementSize;
    list->_element_alignment = element_alignment;
    list->_data              = array_alloc(elementSize * initialCapacity, element_alignment);

    if(list->_data == NULL)
    {
        free(list);
        return NULL;
    }

    return list;
}

void ArrayList_Destroy(ArrayList_t* list)
{
    if(list)
    {
        free(list->_data);
        free(list);
    }
}

static inline int expand_if_needed(ArrayList_t* list, size_t n)
{
    if(list->capacity < n || list->count >= (list->capacity - n))
    {
        const size_t min_expand_size = Utils_Max(ARRAY_LIST_MIN_EXPAND_SIZE, n);
        const size_t new_capacity    = Utils_Max(list->capacity * 2, min_expand_size);

        void* new_data = array_realloc(list->_data, list->capacity * list->_element_size,
                                       new_capacity * list->_element_size, list->_element_alignment);
        if(new_data == NULL)
        {
            return -1;
        }

        list->_data     = new_data;
        list->capacity += min_expand_size;
    }
    return 0;
}

int ArrayList_Realloc(ArrayList_t* list, size_t capacity)
{
    assert(list != NULL);
    assert(capacity > 0);

    void* new_data = array_realloc(list->_data, list->capacity * list->_element_size, capacity * list->_element_size,
                                   list->_element_alignment);
    if(new_data == NULL)
    {
        return -1;
    }

    list->_data    = new_data;
    list->capacity = capacity;

    return 0;
}


int ArrayList_Reserve(ArrayList_t* list, size_t n)
{
    assert(list != NULL);
    assert(n > 0);

    void* new_data = array_realloc(list->_data, list->capacity * list->_element_size,
                                   list->capacity + n * list->_element_size, list->_element_alignment);
    if(new_data == NULL)
    {
        return -1;
    }

    list->_data    = new_data;
    list->capacity = list->capacity + n;

    return 0;
}

int ArrayList_Append(ArrayList_t* list, const void* element)
{
    assert(list != NULL);
    assert(element != NULL);

    if(expand_if_needed(list, 1) != 0)
    {
        return -1;
    }

    memcpy((char*) list->_data + (list->count * list->_element_size), element, list->_element_size);
    list->count++;

    return 0;
}

int ArrayList_Insert(ArrayList_t* list, size_t idx, const void* element)
{
    assert(list != NULL);
    assert(element != NULL);
    assert(idx <= list->count);

    if(expand_if_needed(list, 1) != 0)
    {
        return -1;
    }

    memmove((char*) list->_data + ((idx + 1) * list->_element_size), (char*) list->_data + (idx * list->_element_size),
            (list->count - idx) * list->_element_size);

    memcpy((char*) list->_data + (idx * list->_element_size), element, list->_element_size);
    list->count++;

    return 0;
}

int ArrayList_AppendRange(ArrayList_t* list, const void* elements, size_t count)
{
    assert(list != NULL);
    assert(elements != NULL);
    assert(count > 0);

    if(expand_if_needed(list, count) != 0)
    {
        return -1;
    }

    memcpy((char*) list->_data + (list->count * list->_element_size), elements, count * list->_element_size);
    list->count += count;

    return 0;
}

int ArrayList_InsertRange(ArrayList_t* list, size_t idx, const void* elements, size_t count)
{
    assert(list != NULL);
    assert(elements != NULL);
    assert(count > 0);
    assert(idx <= list->count);

    if(expand_if_needed(list, count) != 0)
    {
        return -1;
    }

    memmove((char*) list->_data + ((idx + count) * list->_element_size),
            (char*) list->_data + (idx * list->_element_size), (list->count - idx) * list->_element_size);

    memcpy((char*) list->_data + (idx * list->_element_size), elements, count * list->_element_size);
    list->count += count;

    return 0;
}

void ArrayList_Remove(ArrayList_t* list, size_t idx)
{
    assert(list != NULL);
    assert(idx < list->count);

    if(idx + 1 == list->count)
    {
        list->count--;
    }

    memmove((char*) list->_data + (idx * list->_element_size), (char*) list->_data + ((idx + 1) * list->_element_size),
            (list->count - idx - 1) * list->_element_size);

    list->count--;
}

void ArrayList_RemoveRange(ArrayList_t* list, size_t idx, size_t count)
{
    assert(list != NULL);
    assert(idx < list->count);
    assert(count > 0);
    assert(idx + count <= list->count);

    if(idx + count == list->count)
    {
        list->count -= count;
        return;
    }

    memmove((char*) list->_data + (idx * list->_element_size),
            (char*) list->_data + ((idx + count) * list->_element_size),
            (list->count - idx - count) * list->_element_size);

    list->count -= count;
}

void ArrayList_Pop(ArrayList_t* list)
{
    assert(list != NULL);

    list->count--;
}

void ArrayList_PopRange(ArrayList_t* list, size_t n)
{
    assert(list != NULL);
    assert(n > 0);
    assert(n <= list->count);

    list->count -= n;
}

void ArrayList_Clear(ArrayList_t* list)
{
    assert(list != NULL);
    list->count = 0;
}

int ArrayList_Trim(ArrayList_t* list)
{
    assert(list != NULL);

    if(list->count == 0 && list->capacity > ARRAY_LIST_MIN_EXPAND_SIZE)
    {
        return ArrayList_Realloc(list, ARRAY_LIST_MIN_EXPAND_SIZE);
    }

    if(list->count < list->capacity / 2)
    {
        return ArrayList_Realloc(list, Utils_Max(list->count * 2, ARRAY_LIST_MIN_EXPAND_SIZE));
    }

    return 0;
}
