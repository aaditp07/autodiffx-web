/*
 * semantic.c - Phase 4: Semantic Analysis
 */
#include <stdio.h>
#include <string.h>
#include "ast.h"

int semantic_check(Node *n)
{
    int errors = 0;
    if (n == NULL) return 0;

    if (n->type == NODE_VAR && strcmp(n->name, "x") != 0) {
        printf("Semantic error: unknown variable '%s' (only x is allowed)\n", n->name);
        errors++;
    }
    if (n->type == NODE_POW && has_x(n->right)) {
        printf("Semantic error: x in an exponent is not supported\n");
        errors++;
    }
    if (n->type == NODE_DIV && is_num(n->right, 0)) {
        printf("Semantic error: division by zero\n");
        errors++;
    }

    errors += semantic_check(n->left);
    errors += semantic_check(n->right);
    return errors;
}
