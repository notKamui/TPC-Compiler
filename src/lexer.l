%{
  /* lexer.l */
  /* Compatible avec parser.y */
  #define MAXNAME 30
  #include "abstract-tree.h"
  #include "parser.tab.h"
  int lineno=1;
  int charno=0;
  int prevno=0;
  char errline[1024];
%}

%option noinput nounput noyywrap
%x SHORT_COMMENT LONG_COMMENT
%%
[ \t\r]+		{charno += yyleng;}
\n			{charno=0; lineno++;}
"/*"			{ charno += yyleng;BEGIN LONG_COMMENT; }
"//"			{ charno += yyleng;BEGIN SHORT_COMMENT; }
&&			{ charno += yyleng;return AND; }
"||"			{ charno += yyleng;return OR; }
"+"|-			{ charno += yyleng; yylval.Operator = yytext[0]; return ADDSUB; }
"*"|"/"|"%"			{ charno += yyleng; yylval.Operator = yytext[0]; return DIVSTAR; }
"<"|"<="|">"|>=		{ charno += yyleng; strcpy(yylval.Compar, yytext); return ORDER; }
==|!=			{ charno += yyleng; strcpy(yylval.Compar, yytext); return EQ; }
int			{ charno += yyleng; strcpy(yylval.Primitive, yytext); return SIMPLETYPE;}
char		{ charno += yyleng; strcpy(yylval.Primitive, yytext); return SIMPLETYPE;}
print			{ charno += yyleng;return PRINT; }
readc			{ charno += yyleng;return READC; }
reade			{ charno += yyleng;return READE; }
void			{ charno += yyleng;return VOID; }
struct		{ charno += yyleng;return STRUCT; }
if			{ charno += yyleng;return IF; }
else			{ charno += yyleng;return ELSE; }
while			{ charno += yyleng;return WHILE; }
return			{ charno += yyleng;return RETURN; }
[a-zA-Z_][a-zA-Z0-9_]*	{ charno += yyleng - 1; strcpy(yylval.Identifier, yytext); return IDENT; }
[0-9]+			{ charno += yyleng; yylval.IntLiteral = atoi(yytext); return NUM;}
'\\?.'			{ 
  charno += yyleng;
  if (yytext[1] == '\\') {
    switch(yytext[2]) {
      case 'n': yylval.CharLiteral = '\n'; break;
      case 't': yylval.CharLiteral = '\t'; break;
      case 'r': yylval.CharLiteral = '\r'; break;
      case 'b': yylval.CharLiteral = '\b'; break;
      case '0': yylval.CharLiteral = '\0'; break;
      case '\'': yylval.CharLiteral = '\''; break;
      case '\"': yylval.CharLiteral = '\"'; break;
      case '\\': yylval.CharLiteral = '\\'; break;
    }
  } else {
    yylval.CharLiteral = yytext[1];
  }
  return CHARACTER;
}
^.*/\n      { strcpy(errline, yytext); REJECT; }
.			{ charno += yyleng; return yytext[0];}
<LONG_COMMENT>"*/"		{ BEGIN INITIAL; charno += yyleng; }
<LONG_COMMENT,SHORT_COMMENT>.		{charno += yyleng;}
<LONG_COMMENT>\n		{lineno++;charno=0;}
<SHORT_COMMENT>\n		{BEGIN INITIAL; lineno++;charno=0;}
%%
