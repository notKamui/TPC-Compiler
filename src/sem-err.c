#include "sem-err.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

int next_line(FILE *f, char *buff) {
    char c, to_string[2];
    assert(f);
    strcpy(buff, "");
    while ((c = fgetc(f))) {
        if (c == EOF) {
            return 0;
        } else if (c == '\n') {
            return 1;
        }
        sprintf(to_string, "%c", c);
        strcat(buff, to_string);
    }
    return 0;
}

void print_err(const char *path, int errtype, int lineno, int charno, char *format, ...) {
    va_list argp;
    char errline[1024], header[1024];
    FILE *file;
    int i;

    file = fopen(path, "r");

    for (i = 0; i < lineno; i++) {
        if (!next_line(file, errline)) {
            break;
        }
    }

    if (errtype == SEM_ERR) {
        strcpy(header, "Semantic error,");
    } else { /* WARNING */
        strcpy(header, "Warning,");
    }

    fprintf(stderr, "%s near line %d char %d: ", header, lineno, charno + 1);
    va_start(argp, format);
    vfprintf(stderr, format, argp);
    va_end(argp);
    fprintf(stderr, "%s\n", errline);

    for (i = 0; i < charno; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");

    fclose(file);
}