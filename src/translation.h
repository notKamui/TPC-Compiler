#ifndef __TRANSLATION__
#define __TRANSLATION__

#include <stdio.h>

#include "abstract-tree.h"
#include "symbols-table.h"

void write_nasm(FILE *f, Node *prog, SymbolsTable *table);

#endif