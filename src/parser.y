%{
  #include <stdio.h>
  #include <string.h>
  #include <unistd.h>
  #include <signal.h>
  #include <fcntl.h>
  #include "abstract-tree.h"
  #include "symbols-table.h"
  #include "translation.h"
  extern int lineno;
  extern int charno;
  extern int yyleng;
  extern char errline[1024];
  int yylex();
  void yyerror(const char *);
  struct Node* rootProg;
  SymbolsTable* table;
  FILE *output;
  char source_filename[64];
  char fname[64];
  int is_anon = 0;
%}


%union {
    struct Node* tree;
    char Identifier[64];
    char CharLiteral;
    int IntLiteral;
    char Compar[3];
    char Primitive[64];
    char Operator;
}

%type <tree> Prog TypesVars Type Declarateurs DeclChamps DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps DeclVars SuiteInstr Instr Exp TB FB M E T F LValue Arguments ListExp

%token <CharLiteral> CHARACTER
%token <IntLiteral> NUM
%token <Identifier> IDENT
%token <Primitive> SIMPLETYPE
%token <Compar> ORDER EQ
%token <Operator> ADDSUB DIVSTAR

%token STRUCT VOID IF WHILE RETURN PRINT READC READE AND OR
%precedence ')'
%precedence ELSE

%%

Prog:
       TypesVars DeclFoncts {$$ = makeNode(Prog); Node* n = makeNode(DeclTypesVars); addChild($$, n); addChild(n, $1); Node* m = makeNode(DeclFoncts); addChild($$, m); addChild(m, $2); rootProg = $$;}
    ;
TypesVars:
       TypesVars Type Declarateurs ';' {$$ = ($1 ? $1 : $2); if ($1) addSibling($$, $2); addChild($2, $3);}
    |  TypesVars STRUCT IDENT '{' DeclChamps '}' ';' {Node *n = makeNode(DeclStruct); $$ = ($1 ? $1 : n); if ($1) addSibling($1, n); strcpy(n->u.identifier, $3); addChild(n, $5);}
    |  %empty  {$$ = NULL;}
    ;
Type:
       SIMPLETYPE {$$ = makeNode(Primitive); strcpy($$->u.identifier, $1);}
    |  STRUCT IDENT {$$ = makeNode(Struct); strcpy($$->u.identifier, $2);}
    ;
Declarateurs:
       Declarateurs ',' IDENT {$$ = $1; Node *n = makeNode(Identifier); addSibling($$, n); strcpy(n->u.identifier, $3);}
    |  IDENT {$$ = makeNode(Identifier); strcpy($$->u.identifier, $1);}
    ;
DeclChamps:
       DeclChamps SIMPLETYPE Declarateurs ';' {$$ = $1; Node *n = makeNode(Primitive); strcpy(n->u.identifier, $2); addSibling($$, n); addChild(n, $3);}
    |  SIMPLETYPE Declarateurs ';' {$$ = makeNode(Primitive); strcpy($$->u.identifier, $1); addChild($$, $2);}
    ;
DeclFoncts:
       DeclFoncts DeclFonct {$$ = $1; addSibling($$, $2);}
    |  DeclFonct {$$ = $1;}
    ;
DeclFonct:
       EnTeteFonct Corps {$$ = makeNode(DeclFonct); addChild($$, $1); addChild($$, $2);}
    ;
EnTeteFonct:
       Type IDENT '(' Parametres ')' {$$ = makeNode(EnTeteFonct); addChild($$, $1); Node* n = makeNode(Identifier); strcpy(n->u.identifier, $2); addChild($$, n); addChild($$, $4);}
    |  VOID IDENT '(' Parametres ')' {$$ = makeNode(EnTeteFonct); addChild($$,  makeNode(Void)); Node* n = makeNode(Identifier); strcpy(n->u.identifier, $2); addChild($$, n); addChild($$, $4);}
    ; 
Parametres:
       VOID {$$ = makeNode(Parametres); addChild($$, makeNode(Void));}
    |  ListTypVar {$$ = makeNode(Parametres); addChild($$, $1);}
    ;
ListTypVar:
       ListTypVar ',' Type IDENT {$$ = $1; addSibling($$, $3); Node* n = makeNode(Identifier); strcpy(n->u.identifier, $4); addChild($3, n);}
    |  Type IDENT {$$ = $1; Node* n = makeNode(Identifier); strcpy(n->u.identifier, $2); addChild($$, n);}
    ;
Corps: 
      '{' DeclVars SuiteInstr '}' {$$ = makeNode(Corps); Node* n = makeNode(DeclVars); addChild($$, n); addChild(n, $2); Node* m = makeNode(SuiteInstr); addChild($$, m); addChild(m, $3);}
    ;
DeclVars:
       DeclVars Type Declarateurs ';' {$$ = ($1 ? $1 : $2); if ($1) addSibling($$, $2); addChild($2, $3);}
    |  %empty {$$ = NULL;}
    ;
SuiteInstr:
       SuiteInstr Instr {$$ = ($1 ? $1 : $2); if ($1) addSibling($$, $2);}
    |  %empty {$$ = NULL;}
    ;
Instr:
       LValue '=' Exp ';' {$$ = makeNode(Assign); addChild($$, $1); addChild($$, $3);}
    |  READE '(' LValue ')' ';' {$$ = makeNode(Reade); addChild($$, $3);}
    |  READC '(' LValue ')' ';' {$$ = makeNode(Readc); addChild($$, $3);}
    |  PRINT '(' Exp ')' ';' {$$ = makeNode(Print); addChild($$, $3);}
    |  IF '(' Exp ')' Instr {$$ = makeNode(If); addChild($$, $3); addChild($$, $5);}
    |  IF '(' Exp ')' Instr ELSE Instr {$$ = makeNode(If); addChild($$, $3); addChild($$, $5); Node *m = makeNode(Else); addChild($$, m); addChild(m, $7);}
    |  WHILE '(' Exp ')' Instr {$$ = makeNode(While); addChild($$, $3); addChild($$, $5);}
    |  Exp ';' {$$ = $1;}
    |  RETURN Exp ';' {$$ = makeNode(Return); addChild($$, $2);}
    |  RETURN ';' {$$ = makeNode(Return); addChild($$, makeNode(Void));}
    |  '{' SuiteInstr '}' {$$ = makeNode(SuiteInstr); addChild($$, $2);}
    |  ';' { $$ = makeNode(EmptyInstr); }
    ;
Exp :  Exp OR TB {$$ = makeNode(Or); addChild($$, $1); addChild($$, $3);}
    |  TB {$$ = $1;}
    ;
TB  :  TB AND FB {$$ = makeNode(And); addChild($$, $1); addChild($$, $3);}
    |  FB {$$ = $1;}
    ;
FB  :  FB EQ M {$$ = makeNode(Compar); strcpy($$->u.identifier, $2); addChild($$, $1); addChild($$, $3);}
    |  M {$$ = $1;}
    ;
M   :  M ORDER E {$$ = makeNode(Compar); strcpy($$->u.identifier, $2); addChild($$, $1); addChild($$, $3);}
    |  E {$$ = $1;}
    ;
E   :  E ADDSUB T {if ($2 == '+') {$$ = makeNode(Plus);} else {$$ = makeNode(Minus);} addChild($$, $1); addChild($$, $3);}
    |  T {$$ = $1;}
    ;
T   :  T DIVSTAR F {if ($2 == '*') {$$ = makeNode(Prod);} else if ($2 == '/') {$$ = makeNode(Div);} else {$$ = makeNode(Mod);} addChild($$, $1); addChild($$, $3);}
    |  F {$$ = $1;}
    ;
F   :  ADDSUB F {if ($1 == '+') {$$ = makeNode(UnaryPlus);} else {$$ = makeNode(UnaryMinus);} addChild($$, $2);}
    |  '!' F {$$ = makeNode(Not); addChild($$, $2);}
    |  '(' Exp ')' {$$ = $2;}
    |  NUM {$$ = makeNode(IntLiteral); $$->u.integer = $1;}
    |  CHARACTER {$$ = makeNode(CharLiteral); $$->u.character = $1;}
    |  LValue {$$ = $1;}
    |  IDENT '(' Arguments ')' {$$ = makeNode(FunctionCall); strcpy($$->u.identifier, $1); addChild($$, $3);}
    ;
LValue:
       IDENT {$$ = makeNode(LValue); Node* n = makeNode(Identifier); strcpy(n->u.identifier, $1); addChild($$, n);}
    |  IDENT '.' IDENT {$$ = makeNode(LValue); Node* n = makeNode(Identifier); sprintf(n->u.identifier, "%s.%s", $1, $3); addChild($$, n);}
    ;
Arguments:
       ListExp {$$ = makeNode(Arguments); addChild($$, $1);}
    |  %empty  {$$ = NULL;}
    ;
ListExp:
       ListExp ',' Exp {$$ = ($1 ? $1 : $3); if ($1) addSibling($$, $3);}
    |  Exp {$$ = $1;}
    ;

%%

void sig_handler(int sig) {
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

int main(int argc, char** argv) {
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
        switch(option) {
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

        strcpy(cut, source_filename);;
        tmp = strtok(cut, "/");
        while (tmp != NULL) {
            tmp = strtok(NULL, "/");
        }
        
        strcpy(fname, strtok(cut, "."));
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
    if (ret) exit(1);
    
    table = create_table(rootProg, source_filename);
    if (showAST) {
        printTree(rootProg);
    }

    if (showTable) {
        print_table(table);
    }
    
    output = fopen(fname ,"w");
    if (output == NULL) {
        fprintf(stderr, "Error: Couldn't write to output file\n");
        delete_table(table);
        raise(SIGUSR2);
    }
    write_nasm(source_filename, output, rootProg, table);
    fclose(output);

    if (withExecutable) {
        sprintf(buf, "nasm -f elf64 %s", fname);
        system(buf);
        sprintf(buf, "gcc -no-pie %s.o -o %s", is_anon ? "_anonymous" : cut, is_anon ? "_anonymous" : cut);
        system(buf);
        sprintf(buf, "rm %s.o", is_anon ? "_anonymous" : cut);
        system(buf);
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

void yyerror(const char *s){
    int i;
    fprintf(stderr, "%s near line %d at char %d\n%s\n", s, lineno, charno-yyleng, errline);
    for (i = 0; i < charno-yyleng; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");
}
