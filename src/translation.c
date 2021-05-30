#include "translation.h"
#include <string.h>

#define COMMENT(s) fprintf(file, "; %s\n", s)
#define LABEL(l) fprintf(file, "%s:\n", l)
#define PUSH(r) fprintf(file, "\tpush %s\n", r)
#define POP(r) fprintf(file, "\tpop %s\n", r)
#define MOV(r1, r2) fprintf(file, "\tmov %s, %s\n", r1, r2)
#define ADD(r1, r2) fprintf(file, "\tadd %s, %s\n", r1, r2)
#define SUB(r1, r2) fprintf(file, "\tsub %s, %s\n", r1, r2)
#define IMUL(r1, r2) fprintf(file, "\timul %s, %s\n", r1, r2)
#define IDIV(r) fprintf(file, "\tidiv %s\n", r)
#define CMP(r1, r2) fprintf(file, "\tcmp %s, %s\n", r1, r2)
#define JMP(l) fprintf(file, "\tjmp %s\n", l)
#define JE(l) fprintf(file, "\tje %s\n", l)
#define SETZ(r) fprintf(file, "\tsetz %s\n", r)
#define SETNZ(r) fprintf(file, "\tsetnz %s\n", r)
#define SETS(r) fprintf(file, "\tsets %s\n", r)
#define SETLE(r) fprintf(file, "\tsetle %s\n", r)
#define SETGE(r) fprintf(file, "\tsetge %s\n", r)
#define AND(r1, r2) fprintf(file, "\tand %s, %s\n", r1, r2)
#define OR(r1, r2) fprintf(file, "\tor %s, %s\n", r1, r2)
#define RET() fprintf(file, "\tret\n")
#define SYSCALL() fprintf(file, "\tsyscall\n")
#define CALL(f) fprintf(file, "\tcall %s\n", f)

static FILE *file;
static unsigned long long labelc;
static int in_main;

static size_t prim_to_size(char *type) {
    if (strcmp("char", type) == 0) {
        return 1;
    } else if (strcmp("int", type) == 0) {
        return 4;
    } else {
        return 0;
    }
}

static char size_to_declsize(size_t size) {
    switch (size) {
        case 1: return 'b';
        case 2: return 'w';
        case 4: return 'd';
        case 8: return 'q';
        default: return 0;
    }
}

/*static char* size_to_asmsize(size_t size) {
    switch (size) {
        case 1: return "BYTE";
        case 2: return "WORD";
        case 4: return "DWORD";
        case 8: return "QWORD";
        case 10: return "TWORD";
        default: return NULL;
    }
} */

static void decl_types(Node *type, SymbolsTable *table) {
    char str_type[10];
    Node *n;
    char tmp[71];
    HashTableIterator it;
    SymbolsTable *t;
    if (type->kind == Primitive) {
        for (n = FIRSTCHILD(type); n; n = n->nextSibling) {
            sprintf(str_type, "res%c", size_to_declsize(prim_to_size(type->u.identifier)));
            fprintf(file, "\t%s: %s 1\n", n->u.identifier, str_type);
        }
    } else if (type->kind == Struct) {
        sprintf(tmp, "struct %s", type->u.identifier);
        t = get_table_by_name(table->parent ? table->parent : table, tmp);
        strcpy(tmp, FIRSTCHILD(type)->u.identifier);
        it = hashtable_iterator_of(t->self);
        while(hashtable_iterator_next(&it))
        {
            sprintf(str_type, "res%c", it.value->type.u.ptype == TPCInt ? 'd' : 'b');
            fprintf(file, "\t%s.%s: %s 1\n", tmp, it.key, str_type);
        }
    }
}

static void bss(Node *type, SymbolsTable *table) {
    Node *n;
    
    fprintf(file, "section .bss\n");
    
    for (n = FIRSTCHILD(type); n; n = n->nextSibling) {
        decl_types(n, table);
    }
    fprintf(file, "\n");
}

static void decl_typevars(Node *typevars, SymbolsTable *table) {
    fprintf(file, "section .data\n");
    fprintf(file, "\tfmtd: db \"%s\", 0\n", "%d");
    fprintf(file, "\tfmtc: db \"%s\", 0\n", "%c");
    fprintf(file, "\n");
    bss(typevars, table);
}

static void instr(Node *self, SymbolsTable *table);

static void binop_pop(Node *op, SymbolsTable *table) {
    instr(FIRSTCHILD(op), table);
    instr(SECONDCHILD(op), table);
    POP("rbx");
    POP("rax");
}

static void bool_op(Node *op, SymbolsTable *table)  {
    switch(op->u.identifier[0]) {
        case '=':
        case '!':
            binop_pop(op, table);
            CMP("rax", "rbx");
            MOV("rax", "0");
            if (op->u.identifier[0] == '=') {
                SETZ("al");
            } else {
                SETNZ("al");
            }
            break;
        case '<':
        case '>':
            instr(FIRSTCHILD(op), table);
            instr(SECONDCHILD(op), table);
            if (op->u.identifier[0] == '<') {
                POP("rbx");
                POP("rax");
            } else {
                POP("rax"); 
                POP("rbx");
            }
            CMP("rax", "rbx");
            MOV("rax", "0");
            if (strlen(op->u.identifier) == 2) {
                SETLE("al");
            } else {
                SETS("al");
            }
            break;
        default:
            break;
    }
    PUSH("rax");
}


static void instr(Node *self, SymbolsTable *table) {
    char tmp[16], tmp2[16];
    int local_label;
    switch (self->kind) {
        case Return:
            COMMENT("stack realign");
            MOV("rsp", "rbp");
            POP("rbp");
            instr(FIRSTCHILD(self), table);
            COMMENT("instr return");
            if (in_main) {
                POP("rdi");
                MOV("rax", "60");
                SYSCALL();
            } else {
                POP("rax");
                RET();
            }
            break;
        case IntLiteral:
            COMMENT("instr int literal");
            sprintf(tmp, "%d", self->u.integer);
            PUSH(tmp);
            break;
        case CharLiteral:
            COMMENT("instr char literal");
            sprintf(tmp, "%d", (int)self->u.character);
            PUSH(tmp);
            break;
        case Plus:
            binop_pop(self, table);
            COMMENT("instr add");
            ADD("rax", "rbx");
            PUSH("rax");
            break;
        case Minus:
            binop_pop(self, table);
            COMMENT("instr sub");
            SUB("rax", "rbx");
            PUSH("rax");
            break;
        case Prod:
            binop_pop(self, table);
            COMMENT("instr mul");
            IMUL("rax", "rbx");
            PUSH("rax");
            break;
        case Div:
            binop_pop(self, table);
            COMMENT("instr div");
            MOV("rdx", "0");
            IDIV("rbx");
            PUSH("rax");
            break;
        case Mod:
            binop_pop(self, table);
            COMMENT("instr mod");
            MOV("rdx", "0");
            IDIV("rbx");
            PUSH("rdx");
            break;
        case UnaryPlus:
            instr(FIRSTCHILD(self), table);
            break;
        case UnaryMinus:
            instr(FIRSTCHILD(self), table);
            COMMENT("instr unary minus");
            POP("rax");
            IMUL("rax", "-1");
            PUSH("rax");
            break;
        case Compar:
            bool_op(self, table);
            break;
        case And:
        case Or:
            binop_pop(self, table);
            COMMENT("bool and/or");
            if (self->kind == And) {
                AND("rax", "rbx");
            } else {
                OR("rax", "rbx");
            }
            MOV("rax", "0");
            SETNZ("al");
            PUSH("rax");
            break;
        case Not:
            instr(FIRSTCHILD(self), table);
            COMMENT("bool not");
            POP("rax");
            CMP("rax", "0");
            MOV("rax", "0");
            SETZ("al");
            PUSH("rax");
            break;
        case If:
            instr(FIRSTCHILD(self), table);
            COMMENT("instr if");
            POP("rax");
            CMP("rax", "0");
            local_label = labelc;
            labelc++;
            sprintf(tmp, "ENDIF%d", local_label);
            sprintf(tmp2, "IF%d", local_label);
            if (THIRDCHILD(self) != NULL && THIRDCHILD(self)->kind == Else) {
                JE(tmp2);
                instr(FIRSTCHILD(SECONDCHILD(self)), table); // instr if
                JMP(tmp);
                LABEL(tmp2);
                instr(FIRSTCHILD(FIRSTCHILD(THIRDCHILD(self))), table); // instr else
                LABEL(tmp);
            } else {
                JE(tmp);
                instr(FIRSTCHILD(SECONDCHILD(self)), table); // instr if
                LABEL(tmp);
            }
            break;
        case While:
            COMMENT("instr while");
            local_label = labelc;
            labelc++;
            sprintf(tmp, "WHILE%d", local_label);
            sprintf(tmp2, "ENDWHILE%d", local_label);
            LABEL(tmp);
            instr(FIRSTCHILD(self), table);
            POP("rax");
            CMP("rax", "0");
            JE(tmp2);
            instr(FIRSTCHILD(SECONDCHILD(self)), table);
            JMP(tmp);
            LABEL(tmp2);
            break;
        case Print:
            instr(FIRSTCHILD(self), table);
            COMMENT("instr print");
            POP("rsi");
            MOV("rax", "0");
            switch (FIRSTCHILD(self)->kind) {
                case CharLiteral:
                    MOV("rdi", "fmtc");
                    break;
                default:
                    MOV("rdi", "fmtd");
                    break;
            }
            CALL("printf");
            break;
        case Readc:
        case Reade:
            instr(FIRSTCHILD(self), table);
        default:
            break;
    }
}

static void suite_instr(Node *instructions, SymbolsTable *table) {
    Node *n;

    for (n = FIRSTCHILD(instructions); n; n = n->nextSibling) {
        instr(n, table);
    }
}

static void decl_fonct(Node *fonct, SymbolsTable *table) {
    char name[64];
    strcpy(name, SECONDCHILD(FIRSTCHILD(fonct))->u.identifier);
    LABEL(name);
    POP("rbp");
    MOV("rbp", "rsp");

    in_main = strcmp(name, "main") == 0;
    suite_instr(SECONDCHILD(SECONDCHILD(fonct)), table);
}

static void decl_foncts(Node *foncts, SymbolsTable *table) {
    Node *n;

    fprintf(file, "section .text\n");
    fprintf(file, "extern printf\n");
    fprintf(file, "extern scanf\n");
    fprintf(file, "global main\n\n");

    for (n = FIRSTCHILD(foncts); n; n = n->nextSibling) {
        
        decl_fonct(n, table);
    }
}

void write_nasm(FILE* f, Node *prog, SymbolsTable *table) {
    file = f;
    labelc = 0;
    
    decl_typevars(FIRSTCHILD(prog), table);
    decl_foncts(SECONDCHILD(prog), table);
}