#include "symbols-table.h"

#include <signal.h>

#include "hashtable.h"
#include "symbol.h"

#define FUNCTION_PARAM 1
#define NOT_FUNCTION_PARAM 0
#define DEFAULT_ARGS_OFFSET 8

/**
 * @brief Deletes and frees a symbol table.
 * @param table the table to be freed.
 */
void delete_table(SymbolsTable *table) {
    SymbolsTable *tmp;

    assert(table);

    hashtable_destroy(table->self);
    for (tmp = table->firstChild; tmp; tmp = tmp->next_sibling) {
        delete_table(tmp); /* frees all*/
    }
    free(table);
}

/**
 * @brief Fetches a subtable in the given table by its name.
 * @param global the parent table (generally the global table).
 * @param name the name of the table to fetch.
 * @return the fetched symbols table.
 */
SymbolsTable *get_table_by_name(SymbolsTable *global, const char *name, int line) {
    SymbolsTable *t;

    assert(global);
    assert(name);

    for (t = global->firstChild; t; t = t->next_sibling) {
        if (strcmp(t->name, name) == 0) {
            return t;
        }
    }
    fprintf(stderr, "Semantic Error, near line %d: unknown symbol '%s'\n", line, name);
    raise(SIGUSR1);
    return NULL;
}

/**
 * @brief Prints a given symbol table.
 * @param table the table to be printed.
 */
void print_table(SymbolsTable *table) {
    SymbolsTable *child;
    HashTableIterator it;

    assert(table);

    it = hashtable_iterator_of(table->self);
    printf("%s", table->name);
    if (table->parent) { /* display table parents */
        printf(" (parent: %s):\n", table->parent->name);
    } else {
        printf(":\n");
    }
    while (hashtable_iterator_next(&it)) {
        printf("\tid: %s   type: %s\t\t\toffset: %d\n", it.key, tpc_to_str(it.value->type), it.value->offset);
    }

    printf("\n");

    for (child = table->firstChild; child; child = child->next_sibling) {
        print_table(child);
    }
}

/**
 * @brief Allocates the memory and creates a symbols table.
 * @param name the name of the table.
 * @return the created table.
*/
static SymbolsTable *init_table(const char *name) {
    SymbolsTable *table;

    assert(name);

    table = malloc(sizeof(struct symbols));

    if (!table) {
        fprintf(stderr, "Error during SymbolsTable allocation, not enough memory\n");
        raise(SIGUSR2);
    }

    strcpy(table->name, name);
    table->parent = NULL;
    table->firstChild = NULL;
    table->next_sibling = NULL;
    table->self = hashtable_new();

    if (!table->self) {
        fprintf(stderr, "Error during HashTable allocation, not enough memory\n");
        raise(SIGUSR2);
    }

    return table;
}

/**
 * @brief Adds a new symbols table as child to an other table.
 * @param parent the parent of the table to create.
 * @param name the name of the table to create.
 * @return the created table.
*/
static SymbolsTable *add_table_child(SymbolsTable *parent, const char name[TABLE_NAME_SIZE]) {
    SymbolsTable *tmp;

    assert(parent);
    assert(name);

    tmp = parent->firstChild; /* stock the former first child */
    parent->firstChild = init_table(name);
    parent->firstChild->next_sibling = tmp;
    parent->firstChild->parent = parent;
    return parent->firstChild;
}

/**
 * @brief Parses an abstract tree node and creates the corresponding tpc type.
 * @param n the node to determine the tpc type.
 * @return the corresponding type.
*/
static TPCType to_tpc(const Node *n) {
    TPCType type;

    assert(n);

    if (strcmp(n->u.identifier, "int") == 0) { /* int type */
        type.u.ptype = TPCInt;
        type.kind = KindPrim;
    } else if (strcmp(n->u.identifier, "char") == 0) { /* char type */
        type.u.ptype = TPCChar;
        type.kind = KindPrim;
    } else if (n->kind == Struct) { /* struct type */
        strcpy(type.u.struct_name, n->u.identifier);
        type.kind = KindStruct;
    } else if (n->kind == DeclStruct) { /* decl struct type */
        type.kind = KindDeclStruct;
    } else { /* error */
        fprintf(stderr, "Unknow type: %s", n->u.identifier);
        exit(3);
    }

    return type;
}

/**
 * @brief Allocates the memory and creates a tpc data.
 * @param table the table where the data will be used.
 * @param type the type of the tpc data.
 * @return the created tpc data.
*/
static TPCData *create_data(SymbolsTable *table, int offset, const TPCType type) {
    TPCData *data;

    assert(table);

    data = (TPCData *)malloc(sizeof(TPCData));
    if (!data) {
        fprintf(stderr, "Error during TPCData allocation, not enough memory\n");
        raise(SIGUSR2);
    }
    data->type = type;
    data->offset = offset;
    return data;
}

/**
 *  @brief Adds a symbol to the table.
 *  @param table the table where to add the symbol.
 *  @param identifier the name of the symbol.
 *  @param data the tpc data of the symbol.
*/
static void add_symbol(SymbolsTable *table, const char identifier[ID_SIZE], TPCData *data, int line) {
    assert(table);
    assert(identifier);
    assert(data);

    if (hashtable_get(table->self, identifier) != NULL) { /* to catch multiple declarations */
        fprintf(stderr, "Semantic Error, near line %d: symbol '%s' already defined\n", line, identifier);
        raise(SIGUSR1);
    }

    if (!hashtable_set(table->self, identifier, data)) {
        fprintf(stderr, "Error during set");
        raise(SIGUSR2);
    }
}

/**
 * @brief Adds a series of symbols to the symbol table.
 * @param table the table where to add the symbols.
 * @param node the current node.
 * @param function_param boolan 1 if in function parameters declaration, 0 otherwise.
 */
static void add_typesvars(SymbolsTable *table, Node *node, int function_param) {
    /* current node = type */
    /* current scope = global / function */
    TPCData *data;
    TPCType type;
    Node *n;
    SymbolsTable *struct_table;
    HashTableIterator it;
    char table_name[TABLE_NAME_SIZE];
    char identifier[2 * ID_SIZE + 1];

    assert(table);
    assert(node);

    if (node->kind == Struct) {
        sprintf(table_name, "%s%s", STRUCT_PREFIX, node->u.identifier); /* creating the name of the struct table */
        struct_table = get_table_by_name(table->parent ? table->parent : table, table_name, node->lineno);
        it = hashtable_iterator_of(struct_table->self);
        while (hashtable_iterator_next(&it)) {                                    /* adding each field of the struct in the table */
            sprintf(identifier, "%s.%s", FIRSTCHILD(node)->u.identifier, it.key); /* writing the struct id */
            data = create_data(table, it.value->offset + table->max_offset, it.value->type);
            data->offset = function_param ? (-data->offset - 16) : data->offset; /* if the symbol represents a func parameter */
            add_symbol(table, identifier, data, node->lineno);
        }
        data = create_data(table, 0, to_tpc(node));
        add_symbol(table, FIRSTCHILD(node)->u.identifier, data, node->lineno);
        table->max_offset += struct_table->max_offset;
    } else {
        type = to_tpc(node);
        for (n = FIRSTCHILD(node); n; n = n->nextSibling) {
            data = create_data(table, table->max_offset, type);
            table->max_offset += type.u.ptype == TPCInt ? 4 : 1;
            data->offset = function_param ? (-data->offset - 16) : data->offset; /* if the symbol represents a func parameter */
            add_symbol(table, n->u.identifier, data, n->lineno);
        }
    }
}

/**
 * @brief Declares a structure and adds its symbols to the symbol table.
 * @param table the table where to add the symbols.
 * @param node the current node.
 */
static void add_declstruct(SymbolsTable *table, const Node *node) {
    /* current node = declstruct */
    /* current table = global */
    Node *n;
    char name[TABLE_NAME_SIZE];
    SymbolsTable *new_scope;

    assert(table);
    assert(node);

    add_symbol(table, node->u.identifier, create_data(table, 0, to_tpc(node)), node->lineno);
    sprintf(name, "%s%s", STRUCT_PREFIX, node->u.identifier);
    new_scope = add_table_child(table, name);           /* create a new scope */
    for (n = FIRSTCHILD(node); n; n = n->nextSibling) { /* add all fields of the struct to the table */
        add_typesvars(new_scope, n, NOT_FUNCTION_PARAM);
    }
}

/**
 * @brief Converts a function declaration into a functionnal type and adds it to the symbol table.
 * @param table the table where to add the function.
 * @param node the current node.
 * @return the functionnal type of the given function, NULL if an problem occured.
 */
static TPCType to_tpc_ftype(SymbolsTable *table, const Node *node) {
    /*  current node = declfonct */
    /* current table = global */
    Node *tmp, *arg;
    TPCType type;
    TPCTypeFunc type_func;

    assert(table);
    assert(node);

    type.kind = KindFunction;
    tmp = FIRSTCHILD(node);                                             /* tmp = EnteteFonct */
    strcpy(type.u.ftype.function_name, SECONDCHILD(tmp)->u.identifier); /* identifier */

    /* return type */
    if (FIRSTCHILD(tmp)->kind == Void) { /* return void */
        type.u.ftype.no_ret = 1;
    } else {
        type.u.ftype.no_ret = 0;
        if (strcmp(FIRSTCHILD(tmp)->u.identifier, "int") == 0) { /* return int */
            type.u.ftype.return_type.kind = KindPrim;
            type.u.ftype.return_type.u.ptype = TPCInt;
        } else if (strcmp(FIRSTCHILD(tmp)->u.identifier, "char") == 0) { /* return char */
            type.u.ftype.return_type.kind = KindPrim;
            type.u.ftype.return_type.u.ptype = TPCChar;
        } else { /* return struct */
            type.u.ftype.return_type.kind = KindStruct;
            strcpy(type.u.ftype.return_type.u.struct_name, FIRSTCHILD(tmp)->u.identifier); /* copy the name of the struct in the return type */
        }
    }

    /* args type */
    tmp = THIRDCHILD(tmp); /* tmp = Parametres */

    type.u.ftype.argc = 0; /* init the function arity */

    if (FIRSTCHILD(tmp)->kind == Void) {
        return type;
    }

    table->max_offset = DEFAULT_ARGS_OFFSET; /* set the default offset for args in the stack */

    for (arg = FIRSTCHILD(tmp); arg; arg = arg->nextSibling) { /* add each arg to the ftable */
        if (type.u.ftype.argc > MAX_ARGS) {
            fprintf(stderr, "Semantic Error, near line %d: too many arguments for '%s'. Limit is 20\n", arg->lineno, (tmp)->u.identifier);
            raise(SIGUSR1);
        }
        if (arg->kind == Primitive) {
            type_func.kind = KindPrim;
            type_func.u.ptype = !strcmp(arg->u.identifier, "int") ? TPCInt : TPCChar;
        } else if (arg->kind == Struct) {
            type_func.kind = KindStruct;
            strcpy(type_func.u.struct_name, arg->u.identifier); /* copy the struct name in the arg type */
        }

        type.u.ftype.args_types[type.u.ftype.argc] = type_func; /* add the arg type to the args list of the functionnal type */
        type.u.ftype.argc++;
        add_typesvars(table, arg, FUNCTION_PARAM);
    }
    table->max_offset = 0;
    return type;
}

/**
 * @brief Adds the sequence of symbols to the symbol table.
 * @param table the table where to add.
 * @param node the current node.
 */
static void add_decltypesvars(SymbolsTable *table, Node *node) {
    /* current node = decltypevars or declvars */
    Node *n;

    assert(table);
    assert(node);

    for (n = FIRSTCHILD(node); n; n = n->nextSibling) { /* n = each declaration */
        switch (n->kind) {
            case Primitive:
            case Struct:
                add_typesvars(table, n, NOT_FUNCTION_PARAM);
                break;
            case DeclStruct:
                add_declstruct(table, n);
                break;
            default:
                fprintf(stderr, "Semantic Error, near line %d: invalid declaration type\n", n->lineno);
                raise(SIGUSR1);
                break;
        }
    }
}

/**
 * @brief Adds a sequence of functions to the symbol table.
 * @param table the table where to add the functions.
 * @param node the current node.
 */
static void add_declfoncts(SymbolsTable *table, Node *node) {
    /* current node = declfoncts */
    /* current table = global */
    Node *n;
    SymbolsTable *child;
    TPCType ftype;
    TPCData *data;
    char buffer[TABLE_NAME_SIZE];

    assert(table);
    assert(node);

    for (n = FIRSTCHILD(node); n; n = n->nextSibling) { /* n = each declfonct of the program */
        sprintf(buffer, "func %s", SECONDCHILD(FIRSTCHILD(n))->u.identifier);
        child = add_table_child(table, buffer);
        ftype = to_tpc_ftype(child, n); /* converts the node to a functionnal type */
        data = create_data(table, 0, ftype);
        add_symbol(table, SECONDCHILD(FIRSTCHILD(n))->u.identifier, data, n->lineno); /* add the function symbol to global table */

        add_decltypesvars(child, FIRSTCHILD(SECONDCHILD(n))); /* add local symbols to the function table */
    }
}

/**
 * @brief Create and allocates a symbol table for the given TPC AST.
 * @param root the root of the AST.
 * @return A pointer to a symbol table. Needs to be freed after use.
 */
SymbolsTable *create_table(Node *root) {
    SymbolsTable *table;
    table = init_table(GLOBAL);

    add_decltypesvars(table, FIRSTCHILD(root));
    add_declfoncts(table, SECONDCHILD(root));

    return table;
}
