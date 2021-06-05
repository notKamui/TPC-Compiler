#include "translation_utils.h"

#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "sem-err.h"

int scope_level;
int flow_count;
int has_control_flow;
Node *suite_instr;

static int rec_check(Node *self);

size_t prim_to_size(char *type) {
    if (strcmp("char", type) == 0) {
        return 1;
    } else if (strcmp("int", type) == 0) {
        return 4;
    } else {
        return 0;
    }
}

char size_to_declsize(size_t size) {
    switch (size) {
        case 1:
            return 'b';
        case 4:
            return 'd';
        default:
            return 0;
    }
}

char *size_to_asmsize(size_t size) {
    switch (size) {
        case 1:
            return "BYTE";
        case 4:
            return "DWORD";
        default:
            return NULL;
    }
}

/* checks recursively if the if else group has a complete return ; returns 1 if is return completed, 0 if not */
static int check_ifelse(Node *n) {
    if (n->kind == SuiteInstr) {
        return rec_check(n);
    } else if (n->kind == Return) {
        return 1;
    } else if (n->kind == If) { /* no braces syntax */
        suite_instr->firstChild = n;
        return rec_check(suite_instr);
    } else {
        return 0;
    }
}

/* starts at SuiteInstr node of Corps ; returns 1 if is return completed, 0 if not */
static int rec_check(Node *self) {
    Node *n;
    int rollback_if, rollback_else;
    int off = 0;

    for (n = FIRSTCHILD(self); n; n = n->nextSibling) {
        switch (n->kind) {
            case If:
                if (THIRDCHILD(n) != NULL) { /* has else ; is ignore if no else branch as it means there has to be a return in the current scope */
                    has_control_flow = 1;
                    scope_level++;
                    flow_count++;
                    off++;

                    rollback_if = check_ifelse(SECONDCHILD(n));
                    rollback_else = check_ifelse(FIRSTCHILD(THIRDCHILD(n)));

                    if (rollback_if && rollback_else) { /* both branches have a return */
                        flow_count--;
                        scope_level--;
                        return 1;
                    } else {
                        scope_level--;
                    }
                } else if (scope_level == 0 && !has_control_flow && n->nextSibling == NULL) {
                    flow_count = -1;
                    return 0;
                }
                break;
            case Return:
                if (scope_level == 0) { /* main scope return is always valid */
                    flow_count = 0;
                    return 1;
                }
                flow_count -= off;
                return 1;
            case SuiteInstr:
                if (rec_check(n)) {
                    if (scope_level == 0) { /* main scope return is always valid */
                        flow_count = 0;
                        return 1;
                    }
                    flow_count -= off;
                    return 1;
                }
                break;
            default:
                if (scope_level == 0 && !has_control_flow && n->nextSibling == NULL) {
                    flow_count = -1;
                    return 0;
                }
                break;
        }
    }
    return 0;
}

/* starts at DeclFonct */
void check_control_flow(const char *source, Node *func) {
    assert(func);

    scope_level = 0;
    flow_count = 0;
    has_control_flow = 0;

    suite_instr = makeNode(SuiteInstr);
    rec_check(SECONDCHILD(SECONDCHILD(func)));
    free(suite_instr);
    if (flow_count != 0) {
        print_err(source, SEM_ERR, func->lineno, func->charno, "control may reach end of non-void function '%s'\n", SECONDCHILD(FIRSTCHILD(func))->u.identifier);
        raise(SIGUSR1);
    }
}