#include <string.h>
#include "hashtable.h"

#define INIT_CAPACITY 32

#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

typedef struct ht_entry {
    const char *key;
    TPCData *value;
} HashTableEntry;

struct ht {
    HashTableEntry *entries;
    size_t capacity;
    size_t length;
};

HashTable *hashtable_new() {
    HashTable *table = malloc(sizeof(HashTable));
    if (table == NULL) {
        return NULL;
    }

    table->length = 0;
    table->capacity = INIT_CAPACITY;

    table->entries = calloc(table->capacity, sizeof(HashTableEntry));
    if (table->entries == NULL) {
        free(table);
        return NULL;
    }

    return table;
}

void hashtable_destroy(HashTable *table) {
    for (size_t i =  0; i < table->capacity; i++) {
        if (table->entries[i].key != NULL) {
            free((void *)table->entries[i].key);
            //free(table->entries[i].value);
        }
    }

    free(table->entries);
    free(table);
}

static u_int64_t hash_key(const char *key) {
    u_int64_t hash = FNV_OFFSET;
    char *p;
    for (p = (char *)key; *p; p++) {
        hash ^= (u_int64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

void *hashtable_get(HashTable *table, const char *key) {
    u_int64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (u_int64_t)(table->capacity - 1));

    while (table->entries[index].key != NULL) {
        if (strcmp(key, table->entries[index].key) == 0) {
            return table->entries[index].value;
        }

        index++;
        if (index >= table->capacity) {
            index = 0;
        }
    }

    return NULL;
}

static const char* hashtable_set_entry(
    HashTableEntry *entries,
    size_t capacity, 
    const char *key,
    TPCData *value,
    size_t *plength
) {
    u_int64_t hash = hash_key(key);
    size_t index = (size_t)(hash & (u_int64_t)(capacity - 1));

    while (entries[index].key != NULL) {
        if (strcmp(key, entries[index].key) == 0) {
            entries[index].value = value;
            return entries[index].key;
        }

        index++;
        if (index >= capacity) {
            index = 0;
        }
    }

    if (plength != NULL) {
        key = strdup(key);
        if (key == NULL) {
            return NULL;
        }
        (*plength)++;
    }
    
    entries[index].key = (char *)key;
    entries[index].value = value;
    return key;
}

static int hashtable_expand(HashTable *table) {
    HashTableEntry *new_entries;
    HashTableEntry entry;
    size_t i;
    size_t new_cap = table->capacity * 2;
    
    if (new_cap < table->capacity) { // overflow
        return 0;
    }

    new_entries = calloc(new_cap, sizeof(HashTableEntry));
    if (new_entries == NULL) {
        return 0;
    }

    for (i = 0; i < table->capacity; i++) {
        entry = table->entries[i];
        if (entry.key != NULL) {
            hashtable_set_entry(new_entries, new_cap, entry.key, entry.value, NULL);
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_cap;
    return 1;
}

const char *hashtable_set(HashTable *table, const char *key, TPCData *value) {
    if (value == NULL) {
        return NULL;
    }

    if (table->length >= table->capacity / 2) {
        if (!hashtable_expand(table)) {
            return NULL;
        }
    }

    return hashtable_set_entry(table->entries, table->capacity, key, value, &table->length);
}

size_t hashtable_length(HashTable *table) {
    return table->length;
}

HashTableIterator hashtable_iterator_of(HashTable *table) {
    HashTableIterator iterator;
    iterator._table = table;
    iterator._index = 0;
    return iterator;
}

int hashtable_iterator_next(HashTableIterator *iterator) {
    size_t i;
    HashTableEntry entry;
    HashTable *table = iterator->_table;

    while (iterator->_index < table->capacity) {
        i = iterator->_index;
        iterator->_index++;

        if (table->entries[i].key != NULL) {
            entry = table->entries[i];
            iterator->key = entry.key;
            iterator->value = entry.value;
            return 1;
        }
    }

    return 0;
}