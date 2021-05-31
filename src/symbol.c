#include "symbol.h"

#include <stdio.h>

static char *tpcf_to_str(TPCTypeFunc ftype) {
    char *buffer = calloc(72, sizeof(char));

    switch (ftype.kind) {
        case KindPrim:
            return ftype.u.ptype == TPCInt ? "int" : "char";
        case KindStruct:
            sprintf(buffer, "struct %s", ftype.u.struct_name);
            return buffer;
        default:
            break;
    }

    return NULL;
}

char *tpc_to_str(TPCType type) {
    char *buffer = calloc(72, sizeof(char));
    int i;

    switch (type.kind) {
        case KindPrim:
            return type.u.ptype == TPCInt ? "int" : "char";
        case KindStruct:
            sprintf(buffer, "struct %s", type.u.struct_name);
            return buffer;
        case KindDeclStruct:
            strcpy(buffer, "struct");
            return buffer;
        case KindFunction:
            strcpy(buffer, "(");

            for (i = 0; i < type.u.ftype.argc; i++) {
                strcat(buffer, tpcf_to_str(type.u.ftype.args_types[i]));
                if (i + 1 < type.u.ftype.argc) {
                    strcat(buffer, ", ");
                }
            }

            if (type.u.ftype.no_ret) {
                strcat(buffer, ") -> void");
            } else {
                strcat(buffer, ") -> ");
                strcat(buffer, tpcf_to_str(type.u.ftype.return_type));
            }
            return buffer;
    }

    return NULL;
}
