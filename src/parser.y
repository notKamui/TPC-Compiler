%{
  #include <stdio.h>
  #include <string.h>
  #include "abstract-tree.h"
  extern int lineno;
  extern int charno;
  extern int yyleng;
  extern char errline[1024];
  void yyerror(const char *);
  struct Node *rootProg;
%}

%code requires {
#include "abstract-tree.h"
typedef char IdentBuf[64];
typedef char ComparBuf[3];
typedef char PrimitiveBuf[64];
}

%define api.value.type union
%start Prog

%type <Node *> Prog TypesVars Type Declarateurs DeclChamps DeclFoncts DeclFonct EnTeteFonct Parametres ListTypVar Corps DeclVars SuiteInstr Instr ElsePart Exp Tb Fb M E T F LValue Arguments ListExp

%token <char> CHARACTER
%token <int> NUM
%token <IdentBuf> IDENT
%token <PrimitiveBuf> SIMPLETYPE
%token <ComparBuf> ORDER EQ
%token <char> ADDSUB DIVSTAR

%token STRUCT VOID IF WHILE RETURN PRINT READC READE AND OR
%precedence THEN
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
    |  IF '(' Exp ')' Instr ElsePart {$$ = makeNode(If); addChild($$, $3); addChild($$, $5); if ($6) addChild($$, $6);}
    |  WHILE '(' Exp ')' Instr {$$ = makeNode(While); addChild($$, $3); addChild($$, $5);}
    |  Exp ';' {$$ = $1;}
    |  RETURN Exp ';' {$$ = makeNode(Return); addChild($$, $2);}
    |  RETURN ';' {$$ = makeNode(Return); addChild($$, makeNode(Void));}
    |  '{' SuiteInstr '}' {$$ = makeNode(SuiteInstr); addChild($$, $2);}
    |  ';' { $$ = makeNode(EmptyInstr); }
    ;
ElsePart:
       ELSE Instr {$$ = makeNode(Else); addChild($$, $2);}
    |  %empty %prec THEN {$$ = NULL;}
    ;
Exp :  Exp OR Tb {$$ = makeNode(Or); addChild($$, $1); addChild($$, $3);}
    |  Tb {$$ = $1;}
    ;
Tb  :  Tb AND Fb {$$ = makeNode(And); addChild($$, $1); addChild($$, $3);}
    |  Fb {$$ = $1;}
    ;
Fb  :  Fb EQ M {$$ = makeNode(Compar); strcpy($$->u.identifier, $2); addChild($$, $1); addChild($$, $3);}
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

void yyerror(const char *s){
    int i;
    fprintf(stderr, "%s near line %d at char %d\n%s\n", s, lineno, charno-yyleng, errline);
    for (i = 0; i < charno-yyleng; i++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "^\n");
}
