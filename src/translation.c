#include "translation.h"

#include <assert.h>
#include <string.h>

#include "translation_utils.h"

#define INT_MAX_DIGITS 16
static unsigned long long labelc;
static int in_main;
static int in_void;

static int instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable);

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
            POP("rax");  // TODO
            COMMENT("stack realign");
            MOV("rsp", "rbp");
            POP("rbp");
        }
        RET();
    }
}

static void litteral_instr(Node *self) {
    char buff[INT_MAX_DIGITS];
    COMMENT("instr literal");
    sprintf(buff, "%d", self->kind == IntLiteral ? self->u.integer : (int)self->u.character);
    PUSH(buff);
}

static void binop_pop(Node *op, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    instr(FIRSTCHILD(op), last_eff_kind, gtable, ftable);
    instr(SECONDCHILD(op), last_eff_kind, gtable, ftable);
    POP("rbx");
    POP("rax");
}

static void int_binop_instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    binop_pop(self, last_eff_kind, gtable, ftable);
    COMMENT("instr int binop");
    switch (self->kind) {
        case Plus:
            ADD("rax", "rbx");
            break;
        case Minus:
            SUB("rax", "rbx");
            break;
        case Prod:
            IMUL("rax", "rbx");
            break;
        case Div:
            MOV("rdx", "0");
            IDIV("rbx");
            break;
        case Mod:
            MOV("rdx", "0");
            IDIV("rbx");
            PUSH("rdx");
            return;
        default:
            break;
    }
    PUSH("rax");
}

static void int_unop_instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    instr(FIRSTCHILD(self), last_eff_kind, gtable, ftable);
    if (self->kind == UnaryMinus) {
        COMMENT("instr unary minus");
        POP("rax");
        NEG("rax");
        PUSH("rax");
    }
}

static void bool_binop_instr(Node *op, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    switch (op->u.identifier[0]) {
        case '=':
        case '!':
            binop_pop(op, last_eff_kind, gtable, ftable);
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
            instr(FIRSTCHILD(op), last_eff_kind, gtable, ftable);
            instr(SECONDCHILD(op), last_eff_kind, gtable, ftable);
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
                SETC("al");
            } else {
                SETNC("al");
            }
            break;
        default:
            break;
    }
    PUSH("rax");
}

static void bool_gate_instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    binop_pop(self, last_eff_kind, gtable, ftable);
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
}

static void not_instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    instr(FIRSTCHILD(self), last_eff_kind, gtable, ftable);
    COMMENT("bool not");
    POP("rax");
    CMP("rax", "0");
    MOV("rax", "0");
    SETZ("al");
    PUSH("rax");
}

static void if_else_instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char endif_label[INT_MAX_DIGITS + 5], else_label[INT_MAX_DIGITS + 4];
    int local_label;

    instr(FIRSTCHILD(self), If, gtable, ftable);
    COMMENT("instr if");
    POP("rax");
    CMP("rax", "0");
    local_label = labelc;
    labelc++;
    sprintf(endif_label, "ENDIF%d", local_label);
    if (THIRDCHILD(self) != NULL && THIRDCHILD(self)->kind == Else) {
        sprintf(else_label, "ELSE%d", local_label);
        JE(else_label);
        instr(SECONDCHILD(self), If, gtable, ftable);  // instr if
        JMP(endif_label);
        LABEL(else_label);
        instr(FIRSTCHILD(THIRDCHILD(self)), Else, gtable, ftable);  // instr else
        LABEL(endif_label);
    } else {
        JE(endif_label);
        instr(SECONDCHILD(self), If, gtable, ftable);  // instr if
        LABEL(endif_label);
    }
}

static void while_instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char while_label[INT_MAX_DIGITS + 5], endwhile_label[INT_MAX_DIGITS + 8];
    int local_label;

    COMMENT("instr while");
    local_label = labelc;
    labelc++;
    sprintf(while_label, "WHILE%d", local_label);
    sprintf(endwhile_label, "ENDWHILE%d", local_label);
    LABEL(while_label);
    instr(FIRSTCHILD(self), While, gtable, ftable);
    POP("rax");
    CMP("rax", "0");
    JE(endwhile_label);
    instr(SECONDCHILD(self), While, gtable, ftable);
    JMP(while_label);
    LABEL(endwhile_label);
}

static void print_instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char id[ID_SIZE];
    TPCData *data;

    instr(FIRSTCHILD(self), Print, gtable, ftable);
    COMMENT("instr print");
    POP("rsi");
    MOV("rax", "0");
    switch (FIRSTCHILD(self)->kind) {
        case CharLiteral:
            MOV("rdi", "fmtc");
            break;
        case LValue:
            strcpy(id, FIRSTCHILD(FIRSTCHILD(self))->u.identifier); /* copy the id of the variable */
            if ((data = hashtable_get(ftable->self, id)) != NULL) { /* local */
                MOV("rdi", data->type.u.ptype == TPCChar ? "fmtc" : "fmtd");
            } else {  // global
                data = hashtable_get(gtable->self, id);
                MOV("rdi", data->type.u.ptype == TPCChar ? "fmtc" : "fmtd");
            }
            break;
        default:
            MOV("rdi", "fmtd");
            break;
    }
    CALL("printf");
}

static void read_instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char buff[ID_SIZE];
    TPCData *data;

    strcpy(buff, FIRSTCHILD(FIRSTCHILD(self))->u.identifier); /* copy the id ov the */
    if ((data = hashtable_get(ftable->self, buff)) != NULL) { /* local */
        MOV("r9", "rsp");
        AND("spl", "240");
        SUB("rsp", "8");
        PUSH("r9");

        sprintf(buff, "[rbp - %d]", data->offset);
        MOV("rdi", self->kind == Reade ? "fmtd" : "fmtc");
        LEA("rsi", buff);
        MOV("rax", "0");
        CALL("scanf");
    } else { /* global */
        data = hashtable_get(gtable->self, buff);
        MOV("rdi", self->kind == Reade ? "fmtd" : "fmtc");
        MOV("rsi", buff);
        MOV("rax", "0");
        CALL("scanf");
    }
}

static void assign_instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char id[ID_SIZE], receiver[16 + INT_MAX_DIGITS + ID_SIZE];
    TPCData *data;

    instr(SECONDCHILD(self), Assign, gtable, ftable);
    COMMENT("instr assignment");
    POP("rax");
    strcpy(id, FIRSTCHILD(FIRSTCHILD(self))->u.identifier);
    if ((data = hashtable_get(ftable->self, id)) != NULL) { /* local */
        sprintf(receiver, "%s [rbp - (%d)]", size_to_asmsize(data->type.u.ptype == TPCInt ? 4 : 1), data->offset);
        MOV(receiver, data->type.u.ptype == TPCInt ? "eax" : "al");
    } else { /* global */
        data = hashtable_get(gtable->self, id);
        sprintf(receiver, "%s [%s]", size_to_asmsize(data->type.u.ptype == TPCInt ? 4 : 1), id);
        MOV(receiver, data->type.u.ptype == TPCInt ? "eax" : "al");
    }
}

static void lvalue_instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char id[64], fetched[128];
    TPCData *data;

    COMMENT("get value");
    strcpy(id, FIRSTCHILD(self)->u.identifier);
    if ((data = hashtable_get(ftable->self, id)) != NULL) { /* local */
        sprintf(fetched, "%s [rbp - (%d)]", size_to_asmsize(data->type.u.ptype == TPCInt ? 4 : 1), data->offset);
        MOV(data->type.u.ptype == TPCInt ? "eax" : "al", fetched);
        PUSH("rax");
    } else { /* global */
        data = hashtable_get(gtable->self, id);
        sprintf(fetched, "%s [%s]", size_to_asmsize(data->type.u.ptype == TPCInt ? 4 : 1), id);
        MOV(data->type.u.ptype == TPCInt ? "eax" : "al", fetched);
        PUSH("rax");
    }
}

static void stack_param(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    assert(self);
    if (self->nextSibling) {
        stack_param(self->nextSibling, gtable, ftable);
    }
    instr(self, FunctionCall, gtable, ftable);
}

static void functioncall_instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    char buff[128];
    TPCData *data;
    SymbolsTable *t;

    MOV("r9", "rsp"); /* saves the old rsp position before params stacking in r9 */

    data = hashtable_get(gtable->self, self->u.identifier);
    if (!data->type.u.ftype.no_ret) {
        if (data->type.u.ftype.return_type.kind == KindPrim) {
            strcpy(buff, data->type.u.ftype.return_type.u.ptype == TPCInt ? "4" : "1");
        } else {
            sprintf(buff, "struct %s", data->type.u.ftype.return_type.u.struct_name);
            t = get_table_by_name(gtable, buff);
            assert(t);
            sprintf(buff, "%d", t->max_offset);
        }
        SUB("rsp", buff); /* allocates the memory for the function return in the stack */
    }
    stack_param(FIRSTCHILD(FIRSTCHILD(self)), gtable, ftable); /* add args of the function on the stack */

    PUSH("r9"); /* push the old rsp position on the top of the stack */
    CALL(self->u.identifier);
    POP("rsp");
    if (last_eff_kind == SuiteInstr) {
        ADD("rsp", buff); /* buff is always the return size here ; disallocates the unusable returned value */
    } else {
        PUSH("rax");
    }
}

static int instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    assert(self);
    assert(gtable);
    assert(ftable);

    switch (self->kind) {
        case SuiteInstr:
            instr(FIRSTCHILD(self), self->kind, gtable, ftable);
            break;
        case Return:
            instr(FIRSTCHILD(self), self->kind, gtable, ftable);
            return_instr();
            return 1;
        /* litteral instr */
        case IntLiteral:
        case CharLiteral:
            if (last_eff_kind == SuiteInstr) break;
            litteral_instr(self);
            break;
        /* int bin op instr*/
        case Plus:
        case Minus:
        case Prod:
        case Div:
        case Mod:
            if (last_eff_kind == SuiteInstr) break;
            int_binop_instr(self, last_eff_kind, gtable, ftable);
            break;
        /* int unary op instr */
        case UnaryPlus:
        case UnaryMinus:
            if (last_eff_kind == SuiteInstr) break;
            int_unop_instr(self, last_eff_kind, gtable, ftable);
            break;
        case Compar:
            if (last_eff_kind == SuiteInstr) break;
            bool_binop_instr(self, last_eff_kind, gtable, ftable);
            break;
        /* bool gate instr */
        case And:
        case Or:
            if (last_eff_kind == SuiteInstr) break;
            bool_gate_instr(self, last_eff_kind, gtable, ftable);
            break;

        case Not:
            if (last_eff_kind == SuiteInstr) break;
            not_instr(self, last_eff_kind, gtable, ftable);
            break;
        case If:
            if_else_instr(self, gtable, ftable);
            break;
        case While:
            while_instr(self, gtable, ftable);
            break;
        case Print:
            print_instr(self, gtable, ftable);
            break;
        /* read instr */
        case Reade:
        case Readc:
            read_instr(self, gtable, ftable);
            break;
        case Assign:
            assign_instr(self, gtable, ftable);
            break;
        case LValue:
            if (last_eff_kind == SuiteInstr) break;
            lvalue_instr(self, gtable, ftable);
            break;
        case FunctionCall:
            functioncall_instr(self, last_eff_kind, gtable, ftable);
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
        end_return = instr(n, SuiteInstr, gtable, ftable);
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