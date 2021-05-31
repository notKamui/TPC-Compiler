#include "symbols-table.h"

#include "hashtable.h"
#include "symbol.h"

static SymbolsTable *init_table(const char *name) {
    assert(name);

    SymbolsTable *table = malloc(sizeof(struct symbols));

    if (!table) {
        fprintf(stderr, "Error during SymbolsTable allocation");
        exit(3);
    }

    strcpy(table->name, name);
    table->parent = NULL;
    table->children = NULL;
    table->next_sibling = NULL;
    table->self = hashtable_new();

    if (!table->self) {
        fprintf(stderr, "Error during HashTable allocation");
        exit(3);
    }

    return table;
}

static SymbolsTable *add_table_child(SymbolsTable *parent, const char *name) {
    SymbolsTable *firstChild;
    firstChild = parent->children;
    parent->children = init_table(name);
    parent->children->next_sibling = firstChild;
    parent->children->parent = parent;
    return parent->children;
}

static TPCType to_tpc(Node *n) {
    assert(n);

    TPCType type;
    if (strcmp(n->u.identifier, "int") == 0) {  // int type
        type.u.ptype = TPCInt;
        type.kind = KindPrim;
    } else if (strcmp(n->u.identifier, "char") == 0) {  // char type
        type.u.ptype = TPCChar;
        type.kind = KindPrim;
    } else if (n->kind == Struct) {  // struct type
        strcpy(type.u.struct_name, n->u.identifier);
        type.kind = KindStruct;
    } else if (n->kind == DeclStruct) {
        type.kind = KindDeclStruct;
    }

    return type;
}

static int type_size(TPCType type) {
    if (type.kind == KindPrim) {
        return type.u.ptype == TPCInt ? 8 : 4;
    } else {
        return 0;
    }
}

static TPCData *create_data(SymbolsTable *table, TPCType type) {
    assert(table);

    TPCData *data;

    data = (TPCData *)malloc(sizeof(TPCData));
    if (!data) {
        return NULL;
    }
    data->type = type;
    data->offset = table->max_offset;
    table->max_offset += type_size(data->type);
    return data;
}

// 1 if everything went ok, 0 otherwise
static int add_var(SymbolsTable *table, const char *identifier, TPCData *data) {
    assert(table);
    assert(identifier);
    assert(data);

    if (hashtable_get(table->self, identifier) != NULL) {
        return 0;
    }

    if (!hashtable_set(table->self, identifier, data)) {
        fprintf(stderr, "Error during set");
        exit(3);
    }
    return 1;
}

SymbolsTable *get_table_by_name(SymbolsTable *global, const char *name) {
    assert(global);
    assert(name);

    SymbolsTable *t;
    for (t = global->children; t; t = t->next_sibling) {
        if (strcmp(t->name, name) == 0) {
            return t;
        }
    }
    // TODO err sem ?
    return NULL;
}

// 1 if everything went ok, 0 otherwise
static int add_typesvars(SymbolsTable *table, Node *node) {
    // current node = type
    assert(table);
    assert(node);

    TPCData *data;
    Node *n;
    TPCType type;
    SymbolsTable *struct_table;
    HashTableIterator it;
    char tmp[71];
    char tmp2[128];

    if (node->kind == Struct) {
        sprintf(tmp, "struct %s", node->u.identifier);
        struct_table =
            get_table_by_name(table->parent ? table->parent : table, tmp);
        sprintf(tmp, "%s.", FIRSTCHILD(node)->u.identifier);
        it = hashtable_iterator_of(struct_table->self);
        while (hashtable_iterator_next(&it)) {
            strcpy(tmp2, tmp);
            strcat(tmp2, it.key);
            data = create_data(table, it.value->type);
            add_var(table, tmp2, data);
        }
    } else {
        type = to_tpc(node);
        for (n = FIRSTCHILD(node); n; n = n->nextSibling) {
            data = create_data(table, type);
            add_var(table, n->u.identifier, data);
        }
    }

    return 1;
}

static int add_declstruct(SymbolsTable *table, Node *node) {
    // current node = declstruct
    // current table = global
    assert(table);
    assert(node);

    Node *n;
    char name[71] = "struct ";
    SymbolsTable *new_scope;

    // add_var(table, node->u.identifier, to_tpc(node));
    strcat(name, node->u.identifier);
    new_scope = add_table_child(table, name);  // create a new scope

    for (n = FIRSTCHILD(node); n;
         n = n->nextSibling) {  // add all fields of the struct to the table
        add_typesvars(new_scope, n);
    }

    return 1;
}

// 1 if everything went ok, 0 otherwise
static TPCType to_tpc_ftype(SymbolsTable *table, Node *n) {
    //  current node = declfonct
    assert(table);
    assert(n);

    Node *tmp, *args;
    TPCType type;
    TPCTypeFunc type_func;

    if (n->kind != DeclFonct) {
        // TODO err sem ?
    }

    type.kind = KindFunction;
    tmp = FIRSTCHILD(n);
    strcpy(type.u.ftype.function_name,
           SECONDCHILD(tmp)->u.identifier);  // identifier

    // return type
    if (FIRSTCHILD(tmp)->kind == Void) {  // return void
        type.u.ftype.no_ret = 1;
    } else {
        type.u.ftype.no_ret = 0;
        if (strcmp(FIRSTCHILD(tmp)->u.identifier, "int") == 0) {  // return int
            type.u.ftype.return_type.kind = KindPrim;
            type.u.ftype.return_type.u.ptype = TPCInt;
        } else if (strcmp(FIRSTCHILD(tmp)->u.identifier, "char") ==
                   0) {  // return char
            type.u.ftype.return_type.kind = KindPrim;
            type.u.ftype.return_type.u.ptype = TPCChar;
        } else {  // return struct
            type.u.ftype.return_type.kind = KindStruct;
            strcpy(type.u.ftype.return_type.u.struct_name,
                   FIRSTCHILD(tmp)->u.identifier);
        }
    }

    // args type
    tmp = THIRDCHILD(tmp);

    type.u.ftype.argc = 0;

    if (FIRSTCHILD(tmp)->kind == Void) {
        return type;
    }

    for (args = FIRSTCHILD(tmp); args; args = args->nextSibling) {
        if (type.u.ftype.argc > MAX_ARGS) {
            // TODO qqch²
        }
        if (args->kind == Primitive) {
            type_func.kind = KindPrim;
            type_func.u.ptype =
                !strcmp(args->u.identifier, "int") ? TPCInt : TPCChar;
        } else if (args->kind == Struct) {
            type_func.kind = KindStruct;
            strcpy(type_func.u.struct_name, args->u.identifier);
        }

        type.u.ftype.args_types[type.u.ftype.argc] = type_func;
        type.u.ftype.argc++;
        if (!add_typesvars(table, args)) {
            // TODO qqch
        }
    }

    return type;
}

// 1 if everything went ok, 0 otherwise
static int add_decltypesvars(SymbolsTable *table, Node *node) {
    // current node = decltypevars or declvars
    assert(table);
    assert(node);

    Node *n;

    for (n = FIRSTCHILD(node); n; n = n->nextSibling) {
        switch (n->kind) {
            case Primitive:
            case Struct:
                if (!add_typesvars(table, n)) {
                    return 0;
                }
                break;
            case DeclStruct:
                if (!add_declstruct(table, n)) {
                    return 0;
                }
                break;
            default:
                return 0;
        }
    }

    return 1;
}

static int add_declfoncts(SymbolsTable *table, Node *node) {
    // current node = declfoncts
    // current table = global
    assert(table);
    assert(node);

    Node *n;
    SymbolsTable *child;
    TPCType type;
    TPCData *data;
    char buffer[69];

    for (n = FIRSTCHILD(node); n; n = n->nextSibling) {
        sprintf(buffer, "func %s", SECONDCHILD(FIRSTCHILD(n))->u.identifier);
        child = add_table_child(table, buffer);
        type = to_tpc_ftype(child, n);
        data = create_data(table, type);
        if (!add_var(table, SECONDCHILD(FIRSTCHILD(n))->u.identifier, data)) {
            return 0;
        }
        if (!add_decltypesvars(child, FIRSTCHILD(SECONDCHILD(n)))) {
            return 0;
        }
    }

    return 1;
}

SymbolsTable *create_table(Node *root) {
    SymbolsTable *table;
    table = init_table(GLOBAL);

    add_decltypesvars(table, FIRSTCHILD(root));
    add_declfoncts(table, SECONDCHILD(root));

    return table;
}

void print_table(SymbolsTable *table) {
    SymbolsTable *child;

    HashTableIterator it = hashtable_iterator_of(table->self);
    printf("%s", table->name);
    if (table->parent) {
        printf(" (parent: %s):\n", table->parent->name);
    } else {
        printf(":\n");
    }
    while (hashtable_iterator_next(&it)) {
        printf("\tid: %s   type: %s\n", it.key, tpc_to_str(it.value->type));
    }

    printf("\n");

    for (child = table->children; child; child = child->next_sibling) {
        print_table(child);
    }
}

void delete_table(SymbolsTable *table) {
    SymbolsTable *tmp;
    hashtable_destroy(table->self);
    for (tmp = table->children; tmp; tmp = tmp->next_sibling) {
        delete_table(tmp);
    }
    free(table);
}