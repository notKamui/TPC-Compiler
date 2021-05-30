#ifndef HASHTABLE
#define HASHTABLE

#include <stdlib.h>
#include "symbol.h"

/**
 * @brief Represents a hash table for const char *key -> void *value
 */
typedef struct ht HashTable;

/**
 * @brief Creates a hash table
 * 
 * @return the new hash table or NULL if issuficient memory
 */
HashTable *hashtable_new();

/**
 * @brief Deletes and frees a hash table
 * 
 * @param table the hash table to be freed
 */
void hashtable_destroy(HashTable *table);

/**
 * @brief Gets a value from a hash table
 * 
 * @param table the hash table to get the value from
 * @param key the key of the value
 * @return the value corresponding to the key (or NULL if it doesn't exist)
 */
void *hashtable_get(HashTable *table, const char *key);

/**
 * @brief Sets a value to a key in a hash table
 * 
 * @param table the table in which to set the value
 * @param key the key to which the value is set
 * @param value the value to set
 * @return the actual key of the value
 */
const char* hashtable_set(HashTable *table, const char *key, TPCData *value);

/**
 * @brief The length of a hash table
 * 
 * @param table the table to get the length
 * @return the length of the table
 */
size_t hashtable_length(HashTable *table);

/**
 * @brief Represents an iterator of a hash table
 */
typedef struct hti {
    const char *key;
    TPCData *value;
    HashTable *_table;
    size_t _index;
} HashTableIterator;

/**
 * @brief Gets an iterator from a hash table
 * 
 * @param table the table to get an iterator from
 * @return the iterator of the table 
 */
HashTableIterator hashtable_iterator_of(HashTable *table);

/**
 * @brief Moves the iterator to the next element of a hash table iterator
 * 
 * @param iterator the iterator which is iterated over
 * @return a truthy value if succesful, and a falsy value otherwise
 */
int hashtable_iterator_next(HashTableIterator *iterator);

#endif