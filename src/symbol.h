#ifndef SYMBOL
#define SYMBOL

#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 20

typedef enum primitive_type { TPCInt, TPCChar } PrimitiveType;

typedef enum tpc_kind {
    KindPrim,
    KindFunction,
    KindStruct,
    KindDeclStruct
} TPCKind;

typedef struct tpct_f {
    TPCKind kind;
    union {
        PrimitiveType ptype;
        char struct_name[64];
    } u;
} TPCTypeFunc;

typedef struct function_type {
    char function_name[64];
    int no_ret;
    TPCTypeFunc return_type;
    int argc;
    TPCTypeFunc args_types[MAX_ARGS];
} FunctionType;

typedef struct tpct {
    TPCKind kind;
    union {
        PrimitiveType ptype;
        FunctionType ftype;
        char struct_name[64];
    } u;
} TPCType;

typedef struct tpcd {
    TPCType type;
    int offset;
} TPCData;

char* tpc_to_str(TPCType type);

#endif