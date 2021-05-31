/* abstract-tree.h */
#ifndef ABSTRACT_TREE
#define ABSTRACT_TREE
typedef enum {
    Prog,
    DeclFoncts,
    DeclFonct,
    EnTeteFonct,
    Parametres,
    Corps,
    DeclVars,
    SuiteInstr,
    LValue,
    Arguments,
    CharLiteral,
    IntLiteral,
    Identifier,
    Struct,
    Primitive,
    Void,
    Assign,
    Reade,
    Readc,
    Print,
    If,
    While,
    Return,
    Else,
    And,
    Or,
    Compar,
    Plus,
    UnaryPlus,
    Minus,
    UnaryMinus,
    Prod,
    Div,
    Mod,
    Not,
    FunctionCall,
    DeclStruct,
    DeclTypesVars,
    EmptyInstr
} Kind;

typedef struct Node {
    Kind kind;
    union {
        int integer;
        char character;
        char identifier[64];
    } u;
    struct Node *firstChild, *nextSibling;
    int lineno;
} Node;

Node *makeNode(Kind kind);
void addSibling(Node *node, Node *sibling);
void addChild(Node *parent, Node *child);
void deleteTree(Node *node);
void printTree(Node *node);

#define FIRSTCHILD(node) (node->firstChild)
#define SECONDCHILD(node) (node->firstChild->nextSibling)
#define THIRDCHILD(node) (node->firstChild->nextSibling->nextSibling)

#endif
