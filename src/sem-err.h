#ifndef __SEM_ERR__
#define __SEM_ERR__

#include <stdio.h>

#define SEM_ERR 0
#define WARNING 1

void print_err(const char *path, int errtype, int lineno, int charno, char *format, ...);

#endif