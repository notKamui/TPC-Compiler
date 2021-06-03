#include "translation.h"

#include <assert.h>
#include <signal.h>
#include <string.h>

#include "sem-err.h"
#include "translation_utils.h"

#define INT_MAX_DIGITS 16
#define INT_TYPE 0
#define CHAR_TYPE 1
#define STRUCT_TYPE 2

FILE *file;
char source_fname[64];
static unsigned long long labelc;
static int in_main;
static size_t current_func_ret_size;
static size_t current_func_args_size;
TPCData *current_func;

static int instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable);

static void decl_types(Node *type, SymbolsTable *gtable) {
    char str_type[10];
    Node *n;
    char tmp[71];
    SymbolsTable *t;

    if (type->kind == Primitive) {
        for (n = FIRSTCHILD(type); n; n = n->nextSibling) {
            sprintf(str_type, "res%c", size_to_declsize(prim_to_size(type->u.identifier)));
            fprintf(file, "\t%s: %s 1\n", n->u.identifier, str_type);
        }
    } else if (type->kind == Struct) {
        sprintf(tmp, "struct %s", type->u.identifier);
        t = get_table_by_name(gtable->parent ? gtable->parent : gtable, tmp, type->lineno, type->charno);

        for (n = FIRSTCHILD(type); n; n = n->nextSibling) {
            fprintf(file, "\t%s: resb %d\n", n->u.identifier, t->max_offset);
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

static void decl_vars(Node *declvars, SymbolsTable *ftable) {
    char offset[10];
    if (ftable->max_offset > 0) {
        COMMENT("local variables declaration");
        sprintf(offset, "%d", ftable->max_offset);
        SUB("rsp", offset);
    }
}

static void stack_return() {
    char stack_label[14 + INT_MAX_DIGITS], end_stack_label[18 + INT_MAX_DIGITS], tmp[27 + INT_MAX_DIGITS];
    int local_label;

    local_label = labelc;
    labelc++;
    sprintf(stack_label, "STACK_RETURN%d", local_label);
    sprintf(end_stack_label, "END_STACK_RETURN%d", local_label);
    sprintf(tmp, "%ld", current_func_ret_size); /* copy the size of the return type */

    COMMENT("add return value in stack");
    MOV("rax", tmp); /* size of return */
    LABEL(stack_label);
    /* stop condition */
    CMP("rax", "0");
    JE(end_stack_label);
    MOV("r9", tmp);
    SUB("r9", "rax");
    MOV("bl", "BYTE [rsp + r9]");
    sprintf(tmp, "%ld", 24 + current_func_ret_size + current_func_args_size);
    MOV("r9", tmp);
    SUB("r9", "rax");
    MOV("BYTE [rbp + r9]", "bl");
    SUB("rax", "1");
    JMP(stack_label);
    LABEL(end_stack_label);
}

static void return_instr() {
    COMMENT("instr return 1st part");
    if (in_main) {
        MOV("edi", "DWORD [rsp]");
        COMMENT("stack realign");
        MOV("rsp", "rbp");
        POP("rbp");
        COMMENT("instr return 2nd part");
        MOV("rax", "60");
        SYSCALL();
    } else {
        if (current_func_ret_size > 0) {
            stack_return();
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
                SETLE("al");
            } else {
                SETL("al");
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
    /* comparison */
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

/* TODO err sem à faire */
static void loc_prim_name(char *fetched, TPCData *data) {
    int fetched_size, data_offset;

    assert(data);
    assert(fetched);

    fetched_size = data->type.u.ptype == TPCInt ? 4 : 1;
    data_offset = data->offset >= 0 ? data->offset + fetched_size : data->offset;
    sprintf(fetched, "%s [rbp - (%d)]", size_to_asmsize(fetched_size), data_offset);
}

/* TODO err sem à faire */
static void glob_prim_name(char *fetched, TPCData *data, const char *id) {
    int fetched_size;
    char buff[2 * ID_SIZE + 1];

    assert(fetched);
    assert(data);

    strcpy(buff, id);
    if (strcmp(id, strtok(buff, ".")) != 0) {
        fetched_size = data->type.u.ptype == TPCInt ? 4 : 1;
        sprintf(fetched, "%s [%s + %d]", size_to_asmsize(fetched_size), buff, data->offset);
    } else {
        fetched_size = data->type.u.ptype == TPCInt ? 4 : 1;
        sprintf(fetched, "%s [%s]", size_to_asmsize(fetched_size), id);
    }
}

static void prim_lvalue_name(char *fetched, Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char id[ID_SIZE];
    TPCData *data;

    strcpy(id, FIRSTCHILD(self)->u.identifier);
    if ((data = hashtable_get(ftable->self, id)) != NULL) { /* local */
        loc_prim_name(fetched, data);
    } else if ((data = hashtable_get(gtable->self, id)) != NULL) { /* global */
        glob_prim_name(fetched, data, id);
    } else { /* not found */
        print_err(source_fname, SEM_ERR, self->lineno, self->charno, "unknown symbol %s\n", id);
        raise(SIGUSR1);
    }
}

static TPCData *get_data(const char *id, SymbolsTable *gtable, SymbolsTable *ftable) {
    TPCData *data;

    data = hashtable_get(ftable->self, id);
    if (data == NULL) {
        data = hashtable_get(gtable->self, id);
    }
    return data;
}

static void compare_types(TPCData *ldata, Node *rvalue, SymbolsTable *gtable, SymbolsTable *ftable) {
    TPCData *rdata;
    char id[ID_SIZE];

    if (rvalue->kind == LValue) {
        strcpy(id, FIRSTCHILD(rvalue)->u.identifier);
    } else {
        strcpy(id, rvalue->u.identifier);
    }
    rdata = get_data(id, gtable, ftable);
    if (rdata == NULL) { /* not found */
        print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "unknown symbol %s\n", id);
        raise(SIGUSR1);
    }
    if (ldata->type.kind == KindPrim) { /* left value type == int or char */
        switch (rdata->type.kind) {
            case KindFunction:
                if (ldata->type.u.ptype == TPCChar && rdata->type.u.ftype.return_type.kind == KindPrim && rdata->type.u.ftype.return_type.u.ptype == TPCInt) {
                    print_err(source_fname, WARNING, rvalue->lineno, rvalue->charno, "implicit cast, expected char, got int\n");
                } else if (rdata->type.u.ftype.return_type.kind != KindPrim) {
                    print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected %s, got struct %s\n", ldata->type.u.ptype == TPCInt ? "int" : "char", rdata->type.u.ftype.return_type.u.struct_name);
                    raise(SIGUSR1);
                }
                break;
            case KindPrim:
                if (ldata->type.u.ptype == TPCChar && rdata->type.u.ptype == TPCInt) {
                    print_err(source_fname, WARNING, rvalue->lineno, rvalue->charno, "implicit cast, expected char, got int\n");
                }
                break;
            default:
                print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected %s, got %s\n", ldata->type.u.ptype == TPCInt ? "int" : "char", rdata->type.u.struct_name);
                raise(SIGUSR1);
                break;
        }
    } else if (ldata->type.kind == KindFunction) {
        if (rdata->type.kind == KindPrim) {
            if (ldata->type.u.ftype.return_type.kind == KindStruct) {
                print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected struct %s, got %s\n", ldata->type.u.ftype.return_type.u.struct_name, rdata->type.u.ptype == TPCInt ? "int" : "char");
                raise(SIGUSR1);
            }
            if (rdata->type.u.ptype == TPCInt && ldata->type.u.ftype.return_type.kind == KindPrim && ldata->type.u.ftype.return_type.u.ptype == TPCChar) {
                print_err(source_fname, WARNING, rvalue->lineno, rvalue->charno, "implicit cast, expected char, got int\n");
            }
        } else {
            if (ldata->type.u.ftype.return_type.kind == KindPrim) {
                print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected %s, got struct %s\n", ldata->type.u.ftype.return_type.u.ptype == TPCInt ? "int" : "char", rdata->type.u.struct_name);
                raise(SIGUSR1);
            }
            if (strcmp(ldata->type.u.ftype.return_type.u.struct_name, rdata->type.u.struct_name) != 0) {
                print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected struct %s, got struct %s\n", ldata->type.u.ftype.return_type.u.struct_name, rdata->type.u.struct_name);
                raise(SIGUSR1);
            }
        }
    } else { /* left value type == struct */
        if (rdata->type.kind == KindPrim) {
            print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected struct %s, got %s\n", ldata->type.u.struct_name, rdata->type.u.ptype == TPCInt ? "int" : "char");
            raise(SIGUSR1);
        } else if (rdata->type.kind == KindStruct) {
            if (strcmp(ldata->type.u.struct_name, rdata->type.u.struct_name) != 0) {
                print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected struct %s, got struct %s\n", ldata->type.u.struct_name, rdata->type.u.struct_name);
                raise(SIGUSR1);
            }
        } else if (rdata->type.kind == KindFunction) { /* right = KindFunction */
            if (rdata->type.u.ftype.return_type.kind == KindStruct) {
                if (strcmp(ldata->type.u.struct_name, rdata->type.u.ftype.return_type.u.struct_name) != 0) {
                    print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected struct %s, got struct %s\n", ldata->type.u.struct_name, rdata->type.u.ftype.return_type.u.struct_name);
                    raise(SIGUSR1);
                }
            } else {
                print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected struct %s, got %s\n", ldata->type.u.struct_name, rdata->type.u.ftype.return_type.u.ptype == TPCInt ? "int" : "char");
                raise(SIGUSR1);
            }
        }
    }
}

static void prim_assign(TPCData *ldata, const char *lvalue_s, Node *rvalue, SymbolsTable *gtable, SymbolsTable *ftable) {
    char rvalue_content[2 * ID_SIZE + 1];
    TPCData *data;

    assert(ldata->type.kind == KindPrim);

    switch (rvalue->kind) {
        case IntLiteral:
            if (ldata->type.u.ptype == TPCChar) {
                print_err(source_fname, WARNING, rvalue->lineno, rvalue->charno, "implicit cast, expected char, got int\n");
            }
            sprintf(rvalue_content, "%d", rvalue->u.integer);
            break;
        case CharLiteral:
            sprintf(rvalue_content, "%d", rvalue->u.character);
            break;
        case FunctionCall:
            if ((data = get_data(rvalue->u.identifier, gtable, ftable)) == NULL) { /* not found */
                print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "unknown symbol %s\n", rvalue->u.identifier);
                raise(SIGUSR1);
            }
            compare_types(ldata, rvalue, gtable, ftable);
            instr(rvalue, Assign, gtable, ftable);
            POP("rax");
            sprintf(rvalue_content, ldata->type.u.ptype == TPCInt ? "eax" : "al");
            break;
        case LValue:
            if ((data = get_data(FIRSTCHILD(rvalue)->u.identifier, gtable, ftable)) == NULL) { /* not found */
                print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "unknown symbol %s\n", FIRSTCHILD(rvalue)->u.identifier);
                raise(SIGUSR1);
            }
            compare_types(ldata, rvalue, gtable, ftable);
            prim_lvalue_name(rvalue_content, rvalue, gtable, ftable);
            MOV(data->type.u.ptype == TPCInt ? "eax" : "al", rvalue_content);
            sprintf(rvalue_content, ldata->type.u.ptype == TPCInt ? "eax" : "al");
            break;
        default:
            instr(rvalue, Assign, gtable, ftable);
            POP("rax");
            sprintf(rvalue_content, "eax");
            break;
    }
    MOV(lvalue_s, rvalue_content);
}

static void struct_assign(TPCData *ldata, Node *rvalue, SymbolsTable *gtable, SymbolsTable *ftable, char *l_global_name) {
    char struct_assign[1 + INT_MAX_DIGITS], end_struct_assign[20 + INT_MAX_DIGITS];
    char buff[TABLE_NAME_SIZE + 16];
    int local_label, offset;
    TPCData *rdata;
    SymbolsTable *struct_table;

    if (rvalue->kind != LValue && rvalue->kind != FunctionCall) {
        print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "invalid type, expected struct %s, got %s\n", ldata->type.u.struct_name, rvalue->kind == IntLiteral ? "int" : "char");
        raise(SIGUSR1);
    }
    compare_types(ldata, rvalue, gtable, ftable);

    local_label = labelc;
    labelc++;
    sprintf(end_struct_assign, "END_STRUCT_ASSIGN%d", local_label);
    sprintf(struct_assign, "STRUCT_ASSIGN%d", local_label);
    COMMENT("struct assign");

    sprintf(buff, "struct %s", ldata->type.u.struct_name);
    struct_table = get_table_by_name(gtable, buff, rvalue->lineno, rvalue->charno);

    MOV("rax", "0"); /* size of the struct */
    LABEL(struct_assign);
    sprintf(buff, "%d", struct_table->max_offset - 1);
    CMP("rax", buff); /* stop condition */
    JG(end_struct_assign);

    if ((rdata = hashtable_get(ftable->self, FIRSTCHILD(rvalue)->u.identifier)) != NULL) { /* local */
        MOV("r9", "rbp");
        sprintf(buff, "%d", rdata->offset);
        if (rdata->offset >= 0) {
            SUB("r9", "rax"); /* move to the next byte to copy */
            SUB("r9", "1");
            SUB("r9", buff);
        } else {
            SUB("r9", "rax");
            SUB("r9", buff);
        }
        /* remove the offset */
        MOV("bl", "BYTE [r9]");                                                                   /* copy the byte in al */
    } else if ((rdata = hashtable_get(gtable->self, FIRSTCHILD(rvalue)->u.identifier)) != NULL) { /* global */
        sprintf(buff, "%d", struct_table->max_offset - 1);
        MOV("r9", buff);
        SUB("r9", "rax");
        sprintf(buff, "BYTE [%s + rax]", FIRSTCHILD(rvalue)->u.identifier); /* remove the offset */
        MOV("bl", buff);
    } else {
        print_err(source_fname, SEM_ERR, rvalue->lineno, rvalue->charno, "unknown symbol %s\n", FIRSTCHILD(rvalue)->u.identifier);
        raise(SIGUSR1);
    }

    if (l_global_name) {
        sprintf(buff, "%d", struct_table->max_offset - 1);
        MOV("r9", buff);
        SUB("r9", "rax");
        sprintf(buff, "BYTE [%s + r9]", l_global_name); /* remove the offset */
        MOV(buff, "bl");
    } else {
        MOV("r9", "rbp");
        sprintf(buff, "%d", ldata->offset);
        if (ldata->offset >= 0) {
            SUB("r9", "rax"); /* move to the next byte to copy */
            SUB("r9", "1");
            SUB("r9", buff);
        } else {
            SUB("r9", "rax");
            SUB("r9", buff);
        }
        MOV("BYTE [r9]", "bl");
    }

    ADD("rax", "1");
    JMP(struct_assign);
    LABEL(end_struct_assign);
}

static void assign_instr(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char lvalue[2 * ID_SIZE + 1], id[ID_SIZE]; /* Changer taille buffer */
    TPCData *data;

    /**
     * si fils == litteral ou lvalue faire un mov
    */

    //instr(SECONDCHILD(self), Assign, gtable, ftable);
    COMMENT("instr assignment");
    strcpy(id, FIRSTCHILD(FIRSTCHILD(self))->u.identifier);
    if ((data = hashtable_get(ftable->self, id)) != NULL) { /* local */
        if (data->type.kind == KindPrim) {
            loc_prim_name(lvalue, data);
            prim_assign(data, lvalue, SECONDCHILD(self), gtable, ftable);
        } else {
            struct_assign(data, SECONDCHILD(self), gtable, ftable, NULL);
        }
    } else if ((data = hashtable_get(gtable->self, id)) != NULL) { /* global */
        if (data->type.kind == KindPrim) {
            glob_prim_name(lvalue, data, id);
            prim_assign(data, lvalue, SECONDCHILD(self), gtable, ftable);
        } else {
            struct_assign(data, SECONDCHILD(self), gtable, ftable, id);
        }
    } else {
    }
}

static void place_on_top(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    char id[64], fetched[128];
    TPCData *data;
    int fetched_size, data_offset;

    strcpy(id, FIRSTCHILD(self)->u.identifier);
    if ((data = hashtable_get(ftable->self, id)) != NULL) { /* local */
        if (data->type.kind == KindPrim) {
            COMMENT("push rvalue");
            fetched_size = data->type.u.ptype == TPCInt ? 4 : 1;
            data_offset = data->offset >= 0 ? data->offset + fetched_size : data->offset;
            sprintf(fetched, "%s [rbp - (%d)]", size_to_asmsize(fetched_size), data_offset);
            MOV(data->type.u.ptype == TPCInt ? "eax" : "al", fetched);
            PUSH("rax");
        } else {
            /** TODO
             * 1 - trouver la taille de la struct et sub rsp de sa taille
             * 2 - faire un while pour mov les bytes 1 par 1
             * 3 - être content d'avoir réussi
            */
        }
    } else if ((data = hashtable_get(gtable->self, id)) != NULL) { /* global */
        if (data->type.kind == KindPrim) {
            glob_prim_name(fetched, data, id);
            MOV(data->type.u.ptype == TPCInt ? "eax" : "al", fetched);
            PUSH("rax");
        }
    } else { /* not found */
        print_err(source_fname, SEM_ERR, self->lineno, self->charno, "unknown symbol %s\n", id);
        raise(SIGUSR1);
    }
}

static int stack_param(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    int offset;
    char buff[INT_MAX_DIGITS + 16];

    assert(self);

    if (self->nextSibling) {
        offset = stack_param(self->nextSibling, gtable, ftable);
    } else {
        offset = 0;
    }

    instr(self, FunctionCall, gtable, ftable);
    switch (self->kind) {
        case CharLiteral:
            MOV("al", "BYTE [rsp]");
            sprintf(buff, "BYTE [rsp + %d]", offset + 7);
            MOV(buff, "al");
            return offset + 7;
        case Struct:
            return offset;
        default:
            MOV("eax", "DWORD [rsp]");
            sprintf(buff, "DWORD [rsp + %d]", offset + 4);
            MOV(buff, "eax");
            return offset + 4;
    }
    return 0;
}

static void functioncall_check(Node *self, TPCData *fdata, SymbolsTable *gtable, SymbolsTable *ftable) {
    int nb_args, n_type;
    TPCData *type_data;
    Node *n;
    TPCTypeFunc type;

    if (FIRSTCHILD(self) == NULL && fdata->type.u.ftype.argc != 0) {
        print_err(source_fname, SEM_ERR, self->lineno, self->charno, "not enough arguments given to function '%s'\n", self->u.identifier);
        raise(SIGUSR1);
    } else if (FIRSTCHILD(self) != NULL && fdata->type.u.ftype.argc == 0) {
        print_err(source_fname, SEM_ERR, self->lineno, self->charno, "too many arguments given to function '%s'\n", self->u.identifier);
        raise(SIGUSR1);
    } else if (FIRSTCHILD(self) == NULL && fdata->type.u.ftype.argc == 0) {
        return;
    }

    nb_args = 0;
    for (n = FIRSTCHILD(FIRSTCHILD(self)); n; n = n->nextSibling) {
        nb_args++;
        if (fdata->type.u.ftype.argc < nb_args) {
            print_err(source_fname, SEM_ERR, self->lineno, self->charno, "too many arguments given to function '%s'\n", self->u.identifier);
            raise(SIGUSR1);
        }
        if (n->kind == LValue) {
            if ((type_data = hashtable_get(ftable->self, FIRSTCHILD(n)->u.identifier)) != NULL) { /* local */
                if (type_data->type.kind == KindPrim) {
                    n_type = type_data->type.u.ptype == TPCInt ? INT_TYPE : CHAR_TYPE;
                } else { /* KindStruct */
                    n_type = STRUCT_TYPE;
                }
            } else { /* global */
                type_data = hashtable_get(gtable->self, FIRSTCHILD(n)->u.identifier);

                if (type_data->type.kind == KindPrim) {
                    n_type = type_data->type.u.ptype == TPCInt ? INT_TYPE : CHAR_TYPE;
                } else { /* KindStruct */
                    n_type = STRUCT_TYPE;
                }
            }
        } else {
            n_type = n->kind == CharLiteral ? CHAR_TYPE : INT_TYPE;
        }
        type = fdata->type.u.ftype.args_types[nb_args - 1];
        if (type.kind == KindPrim) {
            if (type.u.ptype == TPCInt) {
                if (n_type == STRUCT_TYPE) { /* is struct */
                    print_err(source_fname, SEM_ERR, self->lineno, self->charno, "function '%s' argument %d, expected int, got struct %s\n", self->u.identifier, nb_args, type_data->type.u.struct_name);
                    raise(SIGUSR1);
                }
            } else { /* TPCChar */
                if (n->kind == IntLiteral) {
                    print_err(source_fname, WARNING, self->lineno, self->charno, "function '%s' argument %d, expected char, got int\n", self->u.identifier, nb_args);
                } else if (n->kind == LValue) {
                    if (n_type == INT_TYPE) { /* is int */
                        print_err(source_fname, WARNING, self->lineno, self->charno, "function '%s' argument %d, expected char, got int\n", self->u.identifier, nb_args);
                    } else if (n_type == STRUCT_TYPE) { /* is struct */
                        print_err(source_fname, SEM_ERR, self->lineno, self->charno, "function '%s' argument %d, expected char, got struct %s\n", self->u.identifier, nb_args, type_data->type.u.struct_name);
                        raise(SIGUSR1);
                    }
                } else if (n->kind == SuiteInstr) {
                    print_err(source_fname, WARNING, self->lineno, self->charno, "function '%s' argument %d, expected char, got int\n", self->u.identifier, nb_args);
                }
            }
        } else {
            if (n_type == INT_TYPE) {
                print_err(source_fname, SEM_ERR, self->lineno, self->charno, "function '%s' argument %d, expected struct %s, got int\n", self->u.identifier, nb_args, type.u.struct_name);
                raise(SIGUSR1);
            } else if (n_type == CHAR_TYPE) {
                print_err(source_fname, SEM_ERR, self->lineno, self->charno, "function '%s' argument %d, expected struct %s, got char\n", self->u.identifier, nb_args, type.u.struct_name);
                raise(SIGUSR1);
            } else if (strcmp(type_data->type.u.struct_name, type.u.struct_name) != 0) { /* not same struct */
                print_err(source_fname, SEM_ERR, self->lineno, self->charno, "function '%s' argument %d, expected struct %s, got struct %s\n", self->u.identifier, nb_args, type.u.struct_name, type_data->type.u.struct_name);
                raise(SIGUSR1);
            }
        }
    }

    if (fdata->type.u.ftype.argc > nb_args) {
        print_err(source_fname, SEM_ERR, self->lineno, self->charno, "not enough arguments given to function '%s'\n", self->u.identifier);
        raise(SIGUSR1);
    }
}

static void functioncall_instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    char buff[128], buff2[128];
    TPCData *data;
    SymbolsTable *t;
    int offset;

    data = hashtable_get(gtable->self, self->u.identifier);
    assert(data);

    functioncall_check(self, data, gtable, ftable);

    if (!data->type.u.ftype.no_ret) {
        if (data->type.u.ftype.return_type.kind == KindPrim) {
            strcpy(buff, "8");
        } else {
            sprintf(buff, "struct %s", data->type.u.ftype.return_type.u.struct_name);
            t = get_table_by_name(gtable, buff, self->lineno, self->charno);
            assert(t);
            sprintf(buff, "%d", t->max_offset);
        }
        SUB("rsp", buff); /* allocates the memory for the function return in the stack */
    }
    MOV("r9", "rsp"); /* saves the old rsp position before params stacking in r9 */

    if (FIRSTCHILD(self)) {
        offset = stack_param(FIRSTCHILD(FIRSTCHILD(self)), gtable, ftable); /* add args of the function on the stack */

        sprintf(buff2, "%d", offset);
        ADD("rsp", buff2); /* realigning the stack */
    }

    PUSH("r9"); /* push the old rsp position on the top of the stack */
    CALL(self->u.identifier);
    POP("rsp");

    if (last_eff_kind == SuiteInstr && !data->type.u.ftype.no_ret) {
        ADD("rsp", buff); /* buff is always the return size here ; disallocates the unusable returned value */
    }
}

static void return_check(Node *self, SymbolsTable *gtable, SymbolsTable *ftable) {
    if (current_func->type.u.ftype.no_ret && FIRSTCHILD(self) != NULL) {
        print_err(source_fname, SEM_ERR, FIRSTCHILD(self)->lineno, FIRSTCHILD(self)->charno, "unexpected return value in void function\n");
        raise(SIGUSR1);
    }
    switch (FIRSTCHILD(self)->kind) {
        case CharLiteral:
            if (current_func->type.u.ftype.return_type.kind == KindStruct) {
                print_err(source_fname, SEM_ERR, FIRSTCHILD(self)->lineno, FIRSTCHILD(self)->charno, "invalid return type, expected struct %s, got char\n", current_func->type.u.ftype.return_type.u.struct_name);
                raise(SIGUSR1);
            }
            break;
        case LValue:
            compare_types(current_func, FIRSTCHILD(self), gtable, ftable);
            break;
        default: /* IntLiteral or evaluates to int */
            if (current_func->type.u.ftype.return_type.kind == KindPrim) {
                if (current_func->type.u.ftype.return_type.u.ptype == TPCChar) {
                    print_err(source_fname, WARNING, FIRSTCHILD(self)->lineno, FIRSTCHILD(self)->charno, "implicit cast on return type, expected char, got int\n", current_func->type.u.ftype.return_type.u.struct_name);
                }
            } else {
                print_err(source_fname, SEM_ERR, FIRSTCHILD(self)->lineno, FIRSTCHILD(self)->charno, "invalid return type, expected struct %s, got int\n", current_func->type.u.ftype.return_type.u.struct_name);
                raise(SIGUSR1);
            }
            break;
    }
}

static int instr(Node *self, Kind last_eff_kind, SymbolsTable *gtable, SymbolsTable *ftable) {
    assert(self);
    assert(gtable);
    assert(ftable);

    //printf("%d\n", self->kind);

    switch (self->kind) {
        case SuiteInstr:
            instr(FIRSTCHILD(self), self->kind, gtable, ftable);
            break;
        case Return:
            return_check(self, gtable, ftable);
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
            place_on_top(self, gtable, ftable);
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

static size_t return_size(Node *fonct, SymbolsTable *gtable) {
    Node *ret;
    SymbolsTable *table;
    char struct_name[TABLE_NAME_SIZE];

    ret = FIRSTCHILD(FIRSTCHILD(fonct));
    if (ret->kind == Primitive) {
        return prim_to_size(ret->u.identifier);
    } else if (ret->kind == Struct) {
        sprintf(struct_name, "struct %s", ret->u.identifier);
        table = get_table_by_name(gtable, struct_name, ret->lineno, ret->charno);
        assert(table);
        return table->max_offset;
    } else {
        return 0;
    }
}

static void decl_fonct(Node *fonct, SymbolsTable *gtable) {
    char name[69];
    SymbolsTable *ftable;

    assert(gtable);

    sprintf(name, "func %s", SECONDCHILD(FIRSTCHILD(fonct))->u.identifier);
    ftable = get_table_by_name(gtable, name, fonct->lineno, fonct->charno);
    assert(ftable);

    LABEL(&(*(name + 5)));
    PUSH("rbp");
    MOV("rbp", "rsp");

    current_func = hashtable_get(gtable->self, SECONDCHILD(FIRSTCHILD(fonct))->u.identifier);
    assert(current_func);

    // declvars
    decl_vars(FIRSTCHILD(SECONDCHILD(fonct)), ftable);

    // suiteinstr
    in_main = strcmp(name, "func main") == 0;
    current_func_ret_size = return_size(fonct, gtable);
    current_func_args_size = ftable->args_size;
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
        if (n->nextSibling) {
            fprintf(file, "\n");
        }
    }
}

void write_nasm(const char *source, FILE *f, Node *prog, SymbolsTable *gtable) {
    file = f;
    labelc = 0;

    strcpy(source_fname, source);

    decl_typevars(FIRSTCHILD(prog), gtable);
    decl_foncts(SECONDCHILD(prog), gtable);
}