#ifndef __TRANSLATION__
#define __TRANSLATION__

#include "abstract-tree.h"
#include "symbols-table.h"
#include "stdio.h"

void write_nasm(FILE* f, Node *prog, SymbolsTable *table);

#endif