/* abstract-tree.c */
#include "abstract-tree.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int lineno; /* from lexer */

static const char *StringFromKind[] = {
    "Prog", "DeclFoncts", "DeclFonct", "EnTeteFonct", "Parametres",
    "Corps", "DeclVars", "SuiteInstr", "LValue", "Arguments",
    "CharLiteral", "IntLiteral", "Identifier", "Struct", "Primitive",
    "Void", "Assign", "Reade", "Readc", "Print",
    "If", "While", "Return", "Else", "And",
    "Or", "Compar", "Plus", "UnaryPlus", "Minus",
    "UnaryMinus", "Prod", "Div", "Mod", "Not",
    "FunctionCall", "DeclStruct", "DeclTypesVars", "EmptyInstr"};

Node *makeNode(Kind kind) {
    Node *node = malloc(sizeof(Node));
    if (!node) {
        printf("Run out of memory\n");
        exit(1);
    }
    node->kind = kind;
    node->firstChild = node->nextSibling = NULL;
    node->lineno = lineno;
    return node;
}

void addSibling(Node *node, Node *sibling) {
    Node *curr = node;
    while (curr->nextSibling != NULL) {
        curr = curr->nextSibling;
    }
    curr->nextSibling = sibling;
}

void addChild(Node *parent, Node *child) {
    if (parent->firstChild == NULL) {
        parent->firstChild = child;
    } else {
        addSibling(parent->firstChild, child);
    }
}

void deleteTree(Node *node) {
    if (node->firstChild) {
        deleteTree(node->firstChild);
    }
    if (node->nextSibling) {
        deleteTree(node->nextSibling);
    }
    free(node);
}

void printTree(Node *node) {
    static bool rightmost[128];  // current node is rightmost sibling
    static int depth = 0;        // depth of current node
    int i;
    for (i = 1; i < depth; i++) {  // 2502 = vertical line
        printf(rightmost[i] ? "    " : "\u2502   ");
    }
    if (depth > 0) {  // 2514 = up and right; 2500 = horiz; 251c = vertical and right
        printf(rightmost[depth] ? "\u2514\u2500\u2500 "
                                : "\u251c\u2500\u2500 ");
    }

    printf("%s", StringFromKind[node->kind]);
    switch (node->kind) {
        case IntLiteral:
            printf(": %d", node->u.integer);
            break;
        case CharLiteral:
            printf(": ");
            switch (node->u.character) {
                case '\n':
                    printf("'\\n'");
                    break;
                case '\t':
                    printf("'\\t'");
                    break;
                case '\r':
                    printf("'\\r'");
                    break;
                case '\b':
                    printf("'\\b'");
                    break;
                case '\0':
                    printf("'\\0'");
                    break;
                case '\'':
                    printf("'\\\''");
                    break;
                case '\"':
                    printf("'\\\"'");
                    break;
                case '\\':
                    printf("'\\\\'");
                    break;
                default:
                    printf("'%c'", node->u.character);
            }
            break;
        case Identifier:
        case DeclStruct:
        case Primitive:
        case Struct:
        case FunctionCall:
        case Compar:
            printf(": %s", node->u.identifier);
            break;
        default:
            break;
    }
    printf("\n");
    depth++;
    for (Node *child = node->firstChild; child != NULL;
         child = child->nextSibling) {
        rightmost[depth] = (child->nextSibling) ? false : true;
        printTree(child);
    }
    depth--;
}
