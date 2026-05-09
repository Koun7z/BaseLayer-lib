#ifndef BASE_CONFIG_H__
#define BASE_CONFIG_H__

/*
** Array list configuration
*/

/*
** Hash map configuration
*/

// Minimum number of buckets in a hash map, also the default size when creating a hash map with size 0
#define HASH_MAP_MIN_SIZE 16

// How much smaller new data needs to be to prefer reallocation over replacement
#define HASH_MAP_ENTRY_REDUCTION_REALLOC_THRESHOLD 2
#define HASH_MAP_DEFAULT_MAX_LOAD_FACTOR           2.0f
#define HASH_MAP_DEFAULT_MIN_LOAD_FACTOR           (HASH_MAP_DEFAULT_MAX_LOAD_FACTOR / 4.0f)

// Look at HashFunction_t in Base_Hash.h for available hash functions
#define HASH_MAP_HASH_FUNCTION 2

#endif  // BASE_CONFIG_H__