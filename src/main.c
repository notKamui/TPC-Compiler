#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "abstract-tree.h"
#include "parser.tab.h"
#include "symbols-table.h"
#include "translation.h"

extern Node *rootProg;

static SymbolsTable *table;
static FILE *output;
static char source_filename[64];
static char fname[64];
static int is_anon = 0;

static void sig_handler(int sig) {
    deleteTree(rootProg);
    remove(fname);
    if (is_anon) {
        remove(source_filename);
    }
    switch (sig) {
        case SIGUSR1:
            exit(2);
            break;
        case SIGUSR2:
            exit(3);
        default:
            break;
    }
}

int main(int argc, char **argv) {
    int option;
    int showAST = 0;
    int showTable = 0;
    int noOutput = 0;
    int withExecutable = 0;
    char *tmp;
    char cut[64], buf[1024];
    int fno, size;
    FILE *file;

    signal(SIGUSR1, sig_handler);
    signal(SIGUSR2, sig_handler);

    while ((option = getopt(argc, argv, ":tsnxh")) != -1) {
        switch (option) {
            case 't':
                showAST = 1;
                break;
            case 's':
                showTable = 1;
                break;
            case 'n':
                noOutput = 1;
                break;
            case 'x':
                withExecutable = 1;
                break;
            case '?':
                printf("Unknown option\n");
            case 'h':
                printf("Usage: tpcc [OPTIONS] [file]\n\nOPTIONS:\n-t: Prints the AST of the selected program\n-s: Prints the symbol table of the selected program\n-n: No output (prevents creating an asm file)\n-x: produces an executable file\n-h: Prints this message\ntpcc can also receive its input feed from stdin\n");
                return 0;
        }
    }

    if (optind == argc - 1) {
        strcpy(source_filename, argv[optind]);
        file = fopen(source_filename, "r");
        if (file == NULL) {
            fprintf(stderr, "Error: File not found\n");
            exit(3);
        }
        dup2(fileno(file), STDIN_FILENO);
        fclose(file);

        tmp = strrchr(source_filename, '/');
        tmp = tmp ? tmp + 1 : source_filename;
        strncpy(cut, tmp, sizeof(cut) - 1);
        cut[sizeof(cut) - 1] = '\0';
        tmp = strchr(cut, '.');
        if (tmp) {
            *tmp = '\0';
        }
        strcpy(fname, cut);
    } else {
        is_anon = 1;
        strcpy(source_filename, "_anonymous.tpc");
        strcpy(fname, "_anonymous");

        fno = open(source_filename, O_CREAT | O_RDWR | O_TRUNC);
        while ((size = read(STDIN_FILENO, buf, 128)) > 0) {
            write(fno, buf, size);
        }
        close(fno);
        file = fopen(source_filename, "r");
        if (file == NULL) {
            fprintf(stderr, "Error: File not found\n");
            exit(3);
        }
        dup2(fileno(file), STDIN_FILENO);
        fclose(file);
    }
    strcat(fname, ".asm");

    int ret = yyparse();
    if (ret) {
        exit(1);
    }

    table = create_table(rootProg, source_filename);
    if (showAST) {
        printTree(rootProg);
    }

    if (showTable) {
        print_table(table);
    }

    output = fopen(fname, "w");
    if (output == NULL) {
        fprintf(stderr, "Error: Couldn't write to output file\n");
        delete_table(table);
        raise(SIGUSR2);
    }
    write_nasm(source_filename, output, rootProg, table);
    fclose(output);

    if (withExecutable) {
#ifdef __APPLE__
        sprintf(buf, "clang -arch arm64 -x assembler %s -o %s", fname, is_anon ? "_anonymous" : cut);
        system(buf);
#else
        sprintf(buf, "nasm -f elf64 %s", fname);
        system(buf);
        sprintf(buf, "gcc -no-pie %s.o -o %s", is_anon ? "_anonymous" : cut, is_anon ? "_anonymous" : cut);
        system(buf);
        sprintf(buf, "rm %s.o", is_anon ? "_anonymous" : cut);
        system(buf);
#endif
    }

    if (noOutput) {
        remove(fname);
    }

    if (is_anon) {
        remove(source_filename);
    }
    delete_table(table);
    deleteTree(rootProg);

    return 0;
}
