/*
 * semantic.c - Phase 4: Semantic Analysis for AutoDiffX v2
 *
 * Changes from v1:
 *  - Multi-variable: any single-letter variable is valid (x, y, z, etc.)
 *  - Rejects multi-character identifiers that are not function names
 *    (since function names are lexed as FUNC tokens, not ID tokens)
 *  - Improved division-by-zero: uses eval_const() to catch x/(2-2) etc.
 *  - Catches ln(0) and ln(negative constant)
 *  - Catches x^x (variable in exponent) for the differentiation WRT var
 */
#include <stdio.h>
#include <string.h>
#include "ast.h"

static int errors;

static void check(const Node *n, const char *wrt)
{
    double v;

    if (!n) return;

    /* --- Variable validity --- */
    if (n->type == NODE_VAR) {
        /* Only single-letter identifiers are accepted as variables */
        if (strlen(n->name) != 1) {
            printf("Semantic error: '%s' is not a valid variable "
                   "(only single-letter names like x, y, z are allowed)\n",
                   n->name);
            errors++;
        }
        return;
    }

    /* --- Division by zero (constant denominator) --- */
    if (n->type == NODE_DIV && n->right) {
        if (eval_const(n->right, &v) && v == 0.0) {
            printf("Semantic error: division by zero (denominator evaluates to 0)\n");
            errors++;
        }
    }

    /* --- ln domain: ln(c) where c <= 0 --- */
    if (n->type == NODE_FUNC && n->func == FUNC_LN && n->left) {
        if (eval_const(n->left, &v) && v <= 0.0) {
            printf("Semantic error: ln(%.6g) is undefined (argument must be positive)\n", v);
            errors++;
        }
    }

    /* --- Variable in exponent of power rule (w.r.t. wrt variable) ---
     * d(u^v)/d(wrt) with v containing wrt is not supported by the
     * power rule alone (requires logarithmic differentiation).        */
    if (n->type == NODE_POW && n->right && wrt && has_var(n->right, wrt)) {
        printf("Semantic error: exponent contains variable '%s' "
               "(logarithmic differentiation not supported)\n", wrt);
        errors++;
    }

    check(n->left,  wrt);
    check(n->right, wrt);
}

int semantic_check(const Node *n)
{
    errors = 0;
    /* wrt is passed down but semantic_check() signature must stay
       compatible with ast.h; we recover it from the extern below.   */
    extern char wrt_var[];
    check(n, wrt_var);
    return errors;
}
