#include "translation.h"

#include <assert.h>
#include <string.h>

#include "translation_utils.h"

static unsigned long long labelc;
static int in_main;
static int in_void;

static void decl_types(Node *type, SymbolsTable *gtable) {
    char str_type[10];
    Node *n;
    char tmp[71];
    HashTableIterator it;
    SymbolsTable *t;
    if (type->kind == Primitive) {
        for (n = FIRSTCHILD(type); n; n = n->nextSibling) {
            sprintf(str_type, "res%c",
                    size_to_declsize(prim_to_size(type->u.identifier)));
            fprintf(file, "\t%s: %s 1\n", n->u.identifier, str_type);
        }
    } else if (type->kind == Struct) {
        sprintf(tmp, "struct %s", type->u.identifier);
        t = get_table_by_name(gtable->parent ? gtable->parent : gtable, tmp);
        strcpy(tmp, FIRSTCHILD(type)->u.identifier);
        it = hashtable_iterator_of(t->self);
        while (hashtable_iterator_next(&it)) {
            sprintf(str_type, "res%c",
                    it.value->type.u.ptype == TPCInt ? 'd' : 'b');
            fprintf(file, "\t%s.%s: %s 1\n", tmp, it.key, str_type);
        }
    }
}

static void bss(Node *type, SymbolsTable *gtable) {
    Node *n;

    fprintf(file, "section .bss\n");

    for (n = FIRSTCHILD(type); n; n = n->nextSibling) {
        decl_types(n, gtable);
    }
    fprintf(file, "\n");
}

static void decl_typevars(Node *typevars, SymbolsTable *gtable) {
    fprintf(file, "section .data\n");
    fprintf(file, "\tfmtd: db \"%s\", 0\n", "%d");
    fprintf(file, "\tfmtc: db \"%s\", 0\n", "%c");
    fprintf(file, "\n");
    bss(typevars, gtable);
}

static int instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable);

static void binop_pop(Node *op, SymbolsTable *gtable, SymbolsTable *ftable) {
    instr(FIRSTCHILD(op), gtable, ftable);
    instr(SECONDCHILD(op), gtable, ftable);
    POP("rbx");
    POP("rax");
}

static void bool_op(Node *op, SymbolsTable *gtable, SymbolsTable *ftable) {
    switch (op->u.identifier[0]) {
        case '=':
        case '!':
            binop_pop(op, gtable, ftable);
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
            instr(FIRSTCHILD(op), gtable, ftable);
            instr(SECONDCHILD(op), gtable, ftable);
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
                SETNC("al");
            } else {
                SETC("al");
            }
            break;
        default:
            break;
    }
    PUSH("rax");
}

static void return_instr() {
    COMMENT("instr return 1st part");
    if (in_main) {
        POP("rdi");
        COMMENT("stack realign");
        MOV("rsp", "rbp");
        POP("rbp");
        COMMENT("instr return 2nd part");
        MOV("rax", "60");
        SYSCALL();
    } else {
        if (!in_void) {
            POP("rax");
            COMMENT("stack realign");
            MOV("rsp", "rbp");
            POP("rbp");
        }
        RET();
    }
}

static void stack_param(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    assert(self);
    if (self->nextSibling) {
        stack_param(self->nextSibling, gtable, ftable);
    }
    instr(self, gtable, ftable);
}

static int instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char tmp[64], tmp2[128];
    int local_label;
    TPCData *data;
    Kind kind;
    SymbolsTable *t;

    switch (self->kind) {
        case Return:
            instr(FIRSTCHILD(self), gtable, ftable);
            return_instr();
            return 1;
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
            binop_pop(self, gtable, ftable);
            COMMENT("instr add");
            ADD("rax", "rbx");
            PUSH("rax");
            break;
        case Minus:
            binop_pop(self, gtable, ftable);
            COMMENT("instr sub");
            SUB("rax", "rbx");
            PUSH("rax");
            break;
        case Prod:
            binop_pop(self, gtable, ftable);
            COMMENT("instr mul");
            IMUL("rax", "rbx");
            PUSH("rax");
            break;
        case Div:
            binop_pop(self, gtable, ftable);
            COMMENT("instr div");
            MOV("rdx", "0");
            IDIV("rbx");
            PUSH("rax");
            break;
        case Mod:
            binop_pop(self, gtable, ftable);
            COMMENT("instr mod");
            MOV("rdx", "0");
            IDIV("rbx");
            PUSH("rdx");
            break;
        case UnaryPlus:
            instr(FIRSTCHILD(self), gtable, ftable);
            break;
        case UnaryMinus:
            instr(FIRSTCHILD(self), gtable, ftable);
            COMMENT("instr unary minus");
            POP("rax");
            NEG("rax");
            PUSH("rax");
            break;
        case Compar:
            bool_op(self, gtable, ftable);
            break;
        case And:
        case Or:
            binop_pop(self, gtable, ftable);
            if (self->kind == And) {
                COMMENT("bool and");
                CMP("rax", "0");
                MOV("rax", "0");
                SETNZ("al");
                CMP("rbx", "0");
                MOV("rbx", "0");
                SETNZ("bl");
                AND("rax", "rbx");
            } else {
                COMMENT("bool or");
                OR("rax", "rbx");
            }
            MOV("rax", "0");
            SETNZ("al");
            PUSH("rax");
            break;
        case Not:
            instr(FIRSTCHILD(self), gtable, ftable);
            COMMENT("bool not");
            POP("rax");
            CMP("rax", "0");
            MOV("rax", "0");
            SETZ("al");
            PUSH("rax");
            break;
        case If:
            instr(FIRSTCHILD(self), gtable, ftable);
            COMMENT("instr if");
            POP("rax");
            CMP("rax", "0");
            local_label = labelc;
            labelc++;
            sprintf(tmp, "ENDIF%d", local_label);
            sprintf(tmp2, "ELSE%d", local_label);
            if (THIRDCHILD(self) != NULL && THIRDCHILD(self)->kind == Else) {
                JE(tmp2);
                instr(FIRSTCHILD(SECONDCHILD(self)), gtable, ftable);  // instr if
                JMP(tmp);
                LABEL(tmp2);
                instr(FIRSTCHILD(FIRSTCHILD(THIRDCHILD(self))), gtable, ftable);  // instr else
                LABEL(tmp);
            } else {
                JE(tmp);
                instr(FIRSTCHILD(SECONDCHILD(self)), gtable, ftable);  // instr if
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
            instr(FIRSTCHILD(self), gtable, ftable);
            POP("rax");
            CMP("rax", "0");
            JE(tmp2);
            instr(FIRSTCHILD(SECONDCHILD(self)), gtable, ftable);
            JMP(tmp);
            LABEL(tmp2);
            break;
        case Print:
            instr(FIRSTCHILD(self), gtable, ftable);
            COMMENT("instr print");
            POP("rsi");
            MOV("rax", "0");
            kind = FIRSTCHILD(self)->kind;
            switch (kind) {
                case CharLiteral:
                    MOV("rdi", "fmtc");
                    break;
                case LValue:
                    strcpy(tmp, FIRSTCHILD(FIRSTCHILD(self))->u.identifier);
                    if ((data = hashtable_get(ftable->self, tmp)) != NULL) {  // local
                        MOV("rdi", data->type.u.ptype == TPCChar ? "fmtc" : "fmtd");
                    } else {  // global
                        data = hashtable_get(gtable->self, tmp);
                        MOV("rdi", data->type.u.ptype == TPCChar ? "fmtc" : "fmtd");
                    }
                    break;
                default:
                    MOV("rdi", "fmtd");
                    break;
            }
            CALL("printf");
            break;
        case Reade:
        case Readc:
            strcpy(tmp, FIRSTCHILD(FIRSTCHILD(self))->u.identifier);
            if ((data = hashtable_get(ftable->self, tmp)) != NULL) {  // local
                MOV("r9", "rsp");
                AND("spl", "240");
                SUB("rsp", "8");
                PUSH("r9");

                sprintf(tmp2, "[rbp - %d]", data->offset);
                MOV("rdi", self->kind == Reade ? "fmtd" : "fmtc");
                LEA("rsi", tmp2);
                MOV("rax", "0");
                CALL("scanf");
            } else {  // global
                data = hashtable_get(gtable->self, tmp);
                MOV("rdi", self->kind == Reade ? "fmtd" : "fmtc");
                MOV("rsi", tmp);
                MOV("rax", "0");
                CALL("scanf");
            }
            break;
        case Assign:
            instr(SECONDCHILD(self), gtable, ftable);
            COMMENT("instr assignment");
            POP("rax");
            strcpy(tmp, FIRSTCHILD(FIRSTCHILD(self))->u.identifier);
            if ((data = hashtable_get(ftable->self, tmp)) != NULL) {  // local
                sprintf(tmp2, "%s [rbp - (%d)]", size_to_asmsize(data->type.u.ptype == TPCInt ? 4 : 1), data->offset);
                MOV(tmp2, data->type.u.ptype == TPCInt ? "eax" : "al");
            } else {  // global
                data = hashtable_get(gtable->self, tmp);
                sprintf(tmp2, "%s [%s]", size_to_asmsize(data->type.u.ptype == TPCInt ? 4 : 1), tmp);
                MOV(tmp2, data->type.u.ptype == TPCInt ? "eax" : "al");
            }
            break;
        case LValue:
            COMMENT("get value");
            strcpy(tmp, FIRSTCHILD(self)->u.identifier);
            if ((data = hashtable_get(ftable->self, tmp)) != NULL) {  // local
                sprintf(tmp2, "%s [rbp - (%d)]", size_to_asmsize(data->type.u.ptype == TPCInt ? 4 : 1), data->offset);
                MOV(data->type.u.ptype == TPCInt ? "eax" : "al", tmp2);
                PUSH("rax");
            } else {  // global
                data = hashtable_get(gtable->self, tmp);
                sprintf(tmp2, "%s [%s]", size_to_asmsize(data->type.u.ptype == TPCInt ? 4 : 1), tmp);
                MOV(data->type.u.ptype == TPCInt ? "eax" : "al", tmp2);
                PUSH("rax");
            }
            break;
        case FunctionCall:
            MOV("r9", "rsp");

            data = hashtable_get(gtable->self, self->u.identifier);
            if (!data->type.u.ftype.no_ret) {
                if (data->type.u.ftype.return_type.kind == KindPrim) {
                    sprintf(tmp2, "%s", data->type.u.ftype.return_type.u.ptype == TPCInt ? "4" : "1");
                } else {
                    sprintf(tmp2, "struct %s", data->type.u.ftype.return_type.u.struct_name);
                    t = get_table_by_name(gtable, tmp2);
                    sprintf(tmp2, "%d", t->max_offset);
                }
                SUB("rsp", tmp2);
            }
            stack_param(FIRSTCHILD(FIRSTCHILD(self)), gtable, ftable);
            PUSH("r9");
            CALL(self->u.identifier);
            POP("rsp");
            PUSH("rax");
            break;
        default:
            break;
    }
    return 0;
}

static int suite_instr(Node *instructions, SymbolsTable *gtable, SymbolsTable *ftable) {
    Node *n;
    int end_return;

    end_return = 0;
    for (n = FIRSTCHILD(instructions); n; n = n->nextSibling) {
        end_return = instr(n, gtable, ftable);
    }
    return end_return;
}

static void decl_vars(Node *declvars, SymbolsTable *ftable) {
    char offset[10];
    if (ftable->max_offset > 0) {
        COMMENT("local variables declaration");
        sprintf(offset, "%d", ftable->max_offset);
        SUB("rsp", offset);
    }
}

static void decl_fonct(Node *fonct, SymbolsTable *gtable) {
    char name[69];
    SymbolsTable *ftable;
    sprintf(name, "func %s", SECONDCHILD(FIRSTCHILD(fonct))->u.identifier);
    ftable = get_table_by_name(gtable, name);
    assert(ftable);

    LABEL(&(*(name + 5)));
    PUSH("rbp");
    MOV("rbp", "rsp");

    // declvars
    decl_vars(FIRSTCHILD(SECONDCHILD(fonct)), ftable);

    // suiteinstr
    in_main = strcmp(name, "func main") == 0;
    in_void = FIRSTCHILD(FIRSTCHILD(fonct))->kind == Void;
    if (suite_instr(SECONDCHILD(SECONDCHILD(fonct)), gtable, ftable) == 0) {
        COMMENT("stack realign");
        MOV("rsp", "rbp");
        POP("rbp");
        return_instr();
    }
}

static void decl_foncts(Node *foncts, SymbolsTable *gtable) {
    Node *n;

    fprintf(file, "section .text\n");
    fprintf(file, "extern printf\n");
    fprintf(file, "extern scanf\n");
    fprintf(file, "global main\n\n");

    for (n = FIRSTCHILD(foncts); n; n = n->nextSibling) {
        decl_fonct(n, gtable);
        fprintf(file, "\n");
    }
}

void write_nasm(FILE *f, Node *prog, SymbolsTable *gtable) {
    file = f;
    labelc = 0;

    decl_typevars(FIRSTCHILD(prog), gtable);
    decl_foncts(SECONDCHILD(prog), gtable);
}