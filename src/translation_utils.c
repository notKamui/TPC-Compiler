#include "translation_utils.h"

#include <string.h>

size_t prim_to_size(char *type) {
    if (strcmp("char", type) == 0) {
        return 1;
    } else if (strcmp("int", type) == 0) {
        return 4;
    } else {
        return 0;
    }
}

char size_to_declsize(size_t size) {
    switch (size) {
        case 1:
            return 'b';
        case 4:
            return 'd';
        default:
            return 0;
    }
}

char *size_to_asmsize(size_t size) {
    switch (size) {
        case 1:
            return "BYTE";
        case 4:
            return "DWORD";
        default:
            return NULL;
    }
}
