#ifndef SYMBOLS_TABLE
#define SYMBOLS_TABLE

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "abstract-tree.h"
#include "hashtable.h"
#include "symbol.h"

#define GLOBAL "global"

typedef struct symbols {
    char name[32];
    HashTable *self;
    struct symbols *parent;
    struct symbols *children;
    struct symbols *next_sibling;
    int max_offset;
} SymbolsTable;

SymbolsTable* create_table(Node* root);

void delete_table(SymbolsTable *table);

SymbolsTable *get_table_by_name(SymbolsTable *global, const char *name);

void print_table(SymbolsTable* table);

#endif