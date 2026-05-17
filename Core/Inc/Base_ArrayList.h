#ifndef STD_ARRAYLIST_H__
#define STD_ARRAYLIST_H__

#include <stddef.h>

/**
 * @brief Implementation of a dynamic array list in C.
 *        The whole idea of this implementation is to have a dynamic array for any type while still allowing the user
 *        to access the array elements directly using the "[]" operator.*
 * @note  To have access to the array from multiple places you need to store it an "type**", because the "type*" pointer
 *        will change during list resizing.
 *        I'm quite conflicted on this design, it seems nice at first but gets more complicated the more you use it. On
 *        the other hand it retains at least some of the type safety of having normal C array, compared to having to cast
 *        the void* on access.
 *
 * @details The std_arraylist macro creates and array of requested size and automatically places
 *          a header before the first element of this array, then "returns" a properly typed
 *          pointer to the first element of the array. The header can always be found at - sizeof(std_arraylist_header_t)
 *          offset from this pointer. Padding may be inserted if the header size is not a multiple of the array element alignement.
 *
 *          |---------|--------|---------|---------|-----|------------- |
 *          | padding | header | elem[0] | elem[1] | ... | elem[size-1] |
 *          |---------|--------|---------|---------|-----|--------------|
 *                             ^
 *                             | pointer returned by std_arraylist macro
 */

typedef struct
{
    size_t count;
    size_t size;
    size_t element_size;
    size_t header_size;  // including dynamic padding
    char data[];
} ArrayList_Header_t;

/**
 * @brief Allocate an array of a given type and size, and return a typed pointer to the first element of this array.
 *        Following functions will dynamicly modify this array as needed.
 *        Elements of the array can be accessed directly using the "[]" operator, but you should never modify the
 *        array pointer itself directly.
 *        To change the underlying memory allocation, use the ArrayList_* functions.
 *
 * @param _T   : Any type  - Type of the array elements.
 * @param _size: size_t    - Initial size of the array.
 * @return       *_T       - Pointer to the first element of the array.
 *
 */
#define ArrayList(_T, _size) ((_T*) ArrayList_Create(sizeof(_T), _Alignof(_T), _size))

/**
 * @brief Free the memory previously allocated by ArrayList macro.
 *        _array pointer will be set to NULL after this call.
 *
 * @param _array: any** - Pointer to the array pointer returned by ArrayList macro.
 */
#define ArrayList_Free(_array) ArrayList_Destroy((void**) _array)

#define ArrayList_Append(_array, _element) ArrayList_AppendElement((void**) _array, &_element)

#define ArrayList_Insert(_array, _index, _element) ArrayList_InsertElement((void**) _array, _index, &_element)

#define ArrayList_Pop(_array) (ArrayList_get_header(_array)->count--)

#define ArrayList_Remove(_array, _index) ArrayList_RemoveElement(_array, _index, NULL)

#define ArrayList_Clear(_array) ArrayList_ClearList(_array)

#define ArrayList_Trim(_array) ArrayList_TrimList((void**) _array)

#define ArrayList_GetHeader(_array) ((ArrayList_Header_t*) ((char*) _array - sizeof(ArrayList_Header_t)))
#define ArrayList_Count(_array)     (ArrayList_GetHeader(_array)->count)
#define ArrayList_Size(_array)      (ArrayList_GetHeader(_array)->size)


void* ArrayList_Create(size_t elemSize, size_t elemAlignement, size_t size);

void ArrayList_Destroy(void** array);

void* ArrayList_Copy(const void* array);

int ArrayList_Reserve(void** array, size_t numElems);

int ArrayList_Realloc(void** array, size_t newSize);

int ArrayList_AppendElement(void** array, const void* element);

int ArrayList_AppendRange(void** array, const void* elements, size_t numElems);

void ArrayList_PopElement(void* array);

int ArrayList_InsertElement(void** array, size_t index, const void* element);

int ArrayList_InsertRange(void** array, size_t index, const void* elements, size_t numElems);

void ArrayList_RemoveElement(void* array, size_t index);

void ArrayList_RemoveRange(void* array, size_t index, size_t numElems);

void ArrayList_ClearList(void* array);

int ArrayList_TrimList(void** array);

#endif /* STD_ARRAYLIST_H__ */