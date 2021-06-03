#ifndef SYMBOLS_TABLE
#define SYMBOLS_TABLE

#include <assert.h>
#include <stdio.h>

#include "abstract-tree.h"
#include "hashtable.h"
#include "rules.h"
#include "symbol.h"

#define GLOBAL "global"
#define STRUCT_PREFIX "struct "
#define FUNCTION_PREFIX "func "
#define TABLE_NAME_SIZE (7 + ID_SIZE)
/**
 * @brief Represents a symbols table of a part of code
*/
typedef struct symbols {
    char name[TABLE_NAME_SIZE];   /* the name of the table */
    HashTable *self;              /* the hashtable containing the symbols */
    struct symbols *parent;       /* the parent of the table */
    struct symbols *firstChild;   /* the first child of the table */
    struct symbols *next_sibling; /* the next sibling of the table */
    int max_offset;               /* the max offset of the symbols in the stack */
    int args_size;                /* the byte size of the function args */
} SymbolsTable;

/**
 * @brief Creates the symbols table of a program 
 * @param root the root of the abstract tree of the program
 * @return the created symbols table
*/
SymbolsTable *create_table(Node *root, const char *path);

/**
 * @brief Frees the memory used by a symbols table
 * @param table the table to free
*/
void delete_table(SymbolsTable *table);

/**
 * @brief Gets a symbol table thanks to its name
 * @param global the global table of symbols
 * @param name the name of the researched symbols table
 * @return the table if found, NULL otherwise
*/
SymbolsTable *get_table_by_name(SymbolsTable *global, const char *name, int lineno, int charno);

/**
 * @brief Displays a symbols tables
 * @param table the table to display
*/
void print_table(SymbolsTable *table);

#endif