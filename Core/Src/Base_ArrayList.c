#include "Base_ArrayList.h"

#include "Base_Utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define STD_ARRAYLIST_MIN_EXPAND_SIZE 4

void* ArrayList_Create(size_t elemSize, size_t elemAlignement, size_t size)
{
    assert(elemAlignement != 0 && (elemAlignement & (elemAlignement - 1)) == 0);

    // Alight the header with the array elements alignement. The elemAlignement field should always be
    // a power of 2, at least when using the std_arraylist macro.
    const size_t header_size = sizeof(ArrayList_Header_t)
                             + (elemAlignement - (sizeof(ArrayList_Header_t) & (elemAlignement - 1)));

    char* array = NULL;
    if(elemAlignement > _Alignof(max_align_t))
    {
        array = (char*) aligned_alloc(elemAlignement, header_size + elemSize * size);
    }
    else
    {
        array = (char*) malloc(header_size + elemSize * size);
    }

    if(array)
    {
        ArrayList_Header_t* header = (ArrayList_Header_t*) (array + (header_size - sizeof(ArrayList_Header_t)));

        header->count        = 0;
        header->size         = size;
        header->element_size = elemSize;
        header->header_size  = header_size;

        return (void*) (array + header_size);
    }

    return NULL;
}

void ArrayList_Destroy(void** array)
{
    if(*array == NULL)
    {
        return;
    }

    ArrayList_Header_t* header = ArrayList_GetHeader(*array);
    free((char*) *array - header->header_size);
    *array = NULL;
}

int ArrayList_Reserve(void** array, size_t numElems)
{
    ArrayList_Header_t* header = ArrayList_GetHeader(*array);

    char* new_array = (char*) realloc((char*) *array - header->header_size,
                                      header->header_size + header->element_size * (header->size + numElems));
    if(new_array)
    {
        *array        = (void*) (new_array + header->header_size);
        header        = (ArrayList_Header_t*) (*array - sizeof(ArrayList_Header_t));
        header->size += numElems;
        return 0;
    }
    return -1;
}

int ArrayList_Realloc(void** array, size_t newSize)
{
    ArrayList_Header_t* header = ArrayList_GetHeader(*array);

    char* new_array = (char*) realloc((char*) *array - header->header_size,
                                      header->header_size + header->element_size * newSize);
    if(new_array)
    {
        *array       = (void*) (new_array + header->header_size);
        header       = (ArrayList_Header_t*) (*array - sizeof(ArrayList_Header_t));
        header->size = newSize;
        return 0;
    }
    return -1;
}

static inline int expand_if_needed(void** array, ArrayList_Header_t** header)
{
    if((*header)->count >= (*header)->size)
    {
        // Expand the array by 12.5% (1/8) when it's full. TODO: find good expansion factor.
        int status = ArrayList_Reserve(array, Utils_Max((*header)->size >> 3, STD_ARRAYLIST_MIN_EXPAND_SIZE));
        if(status == 0)
        {
            *header = ArrayList_GetHeader(*array);
        }
        return status;
    }
    return 0;
}

int ArrayList_AppendElement(void** array, const void* element)
{
    ArrayList_Header_t* header = ArrayList_GetHeader(*array);

    if(expand_if_needed(array, &header))
    {
        return -1;
    }

    memcpy((char*) *array + header->count * header->element_size, element, header->element_size);
    header->count++;

    return 0;
}

int ArrayList_InsertElement(void** array, size_t index, const void* element)
{
    ArrayList_Header_t* header = ArrayList_GetHeader(*array);

    if(index > header->count)
    {
        return -1;
    }

    if(expand_if_needed(array, &header))
    {
        return -1;
    }

    memmove((char*) *array + (index + 1) * header->element_size, (char*) *array + index * header->element_size,
            (header->count - index) * header->element_size);
    memcpy((char*) *array + index * header->element_size, element, header->element_size);
    header->count++;
    return 0;
}

void ArrayList_PopElement(void* array)
{
    ArrayList_Header_t* header = ArrayList_GetHeader(array);

    if(header->count > 0)
    {
        header->count--;
    }
}

void ArrayList_RemoveElement(void* array, size_t index)
{
    ArrayList_Header_t* header = ArrayList_GetHeader(array);

    if(index >= header->count)
    {
        return;
    }

    memmove((char*) array + index * header->element_size, (char*) array + (index + 1) * header->element_size,
            (header->count - index - 1) * header->element_size);
    header->count--;
}

void ArrayList_ClearList(void* array)
{
    ArrayList_Header_t* header = ArrayList_GetHeader(array);
    header->count              = 0;
}

int ArrayList_TrimList(void** array)
{
    const ArrayList_Header_t* header = ArrayList_GetHeader(*array);
    return ArrayList_Realloc(array, header->count);
}