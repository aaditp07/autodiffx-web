/*
 * ast.c - Phase 3: Abstract Syntax Tree construction, printing, and memory.
 *
 * Changes from v1:
 *  - NODE_FUNC support (sin, cos, tan, ln, exp)
 *  - Multi-variable: has_var(n, "y") checks for specific variable
 *  - is_const_wrt(n, var): true when expression has no occurrence of var
 *  - eval_const: attempts full constant evaluation (used in semantic checks)
 *  - copy_node: deep copy (needed by diff.c to avoid aliasing bugs)
 *  - free_node: recursive memory cleanup
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ast.h"

/* ------------------------------------------------------------------ */
/* Constructors                                                         */
/* ------------------------------------------------------------------ */

static Node *new_node(NodeType type)
{
    Node *n = calloc(1, sizeof(Node));
    if (!n) { fprintf(stderr, "Out of memory\n"); exit(1); }
    n->type = type;
    return n;
}

Node *new_num(double value)
{
    Node *n = new_node(NODE_NUM);
    n->value = value;
    return n;
}

Node *new_var(char *name)
{
    Node *n = new_node(NODE_VAR);
    n->name = strdup(name);
    return n;
}

Node *new_op(NodeType type, Node *left, Node *right)
{
    Node *n = new_node(type);
    n->left  = left;
    n->right = right;
    return n;
}

Node *new_neg(Node *operand)
{
    return new_op(NODE_NEG, operand, NULL);
}

Node *new_func(FuncType f, Node *arg)
{
    Node *n = new_node(NODE_FUNC);
    n->func = f;
    n->left = arg;   /* arg is stored as left child */
    return n;
}

/* Deep copy — needed in diff.c to avoid pointer aliasing */
Node *copy_node(const Node *n)
{
    Node *c;
    if (!n) return NULL;
    c = new_node(n->type);
    c->value = n->value;
    c->func  = n->func;
    c->name  = n->name ? strdup(n->name) : NULL;
    c->left  = copy_node(n->left);
    c->right = copy_node(n->right);
    return c;
}

/* ------------------------------------------------------------------ */
/* Memory management                                                    */
/* ------------------------------------------------------------------ */

void free_node(Node *n)
{
    if (!n) return;
    free_node(n->left);
    free_node(n->right);
    if (n->name) free(n->name);
    free(n);
}

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

int is_num(const Node *n, double value)
{
    return n && n->type == NODE_NUM && n->value == value;
}

/* Does node n contain variable 'var'? */
int has_var(const Node *n, const char *var)
{
    if (!n) return 0;
    if (n->type == NODE_VAR && strcmp(n->name, var) == 0) return 1;
    return has_var(n->left, var) || has_var(n->right, var);
}

/* Is n free of variable 'var'? (treat as constant w.r.t. var) */
int is_const_wrt(const Node *n, const char *var)
{
    return !has_var(n, var);
}

/*
 * Attempt to evaluate n as a fully-numeric constant.
 * Returns 1 on success (result in *out), 0 if n contains variables.
 */
int eval_const(const Node *n, double *out)
{
    double l, r;
    if (!n) return 0;
    switch (n->type) {
    case NODE_NUM: *out = n->value; return 1;
    case NODE_VAR: return 0;
    case NODE_NEG:
        if (!eval_const(n->left, &l)) return 0;
        *out = -l; return 1;
    case NODE_FUNC:
        if (!eval_const(n->left, &l)) return 0;
        switch (n->func) {
        case FUNC_SIN: *out = sin(l); return 1;
        case FUNC_COS: *out = cos(l); return 1;
        case FUNC_TAN: *out = tan(l); return 1;
        case FUNC_LN:  if (l <= 0) return 0; *out = log(l); return 1;
        case FUNC_EXP: *out = exp(l); return 1;
        }
        return 0;
    default:
        if (!eval_const(n->left, &l) || !eval_const(n->right, &r)) return 0;
        switch (n->type) {
        case NODE_ADD: *out = l + r; return 1;
        case NODE_SUB: *out = l - r; return 1;
        case NODE_MUL: *out = l * r; return 1;
        case NODE_DIV: if (r == 0) return 0; *out = l / r; return 1;
        case NODE_POW: *out = pow(l, r); return 1;
        default: return 0;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Printing                                                             */
/* ------------------------------------------------------------------ */

static const char *func_name(FuncType f)
{
    switch (f) {
    case FUNC_SIN: return "sin";
    case FUNC_COS: return "cos";
    case FUNC_TAN: return "tan";
    case FUNC_LN:  return "ln";
    case FUNC_EXP: return "exp";
    }
    return "?";
}

/* Operator precedence for bracket decisions */
static int precedence(const Node *n)
{
    if (!n) return 5;
    switch (n->type) {
    case NODE_ADD: case NODE_SUB: return 1;
    case NODE_MUL: case NODE_DIV: return 2;
    case NODE_NEG:                return 3;
    case NODE_POW:                return 4;
    case NODE_NUM: return (n->value < 0) ? 3 : 5;
    default:                      return 5;
    }
}

static void print_part(const Node *n, int need_brackets)
{
    if (need_brackets) printf("(");
    print_expr(n);
    if (need_brackets) printf(")");
}

void print_expr(const Node *n)
{
    int p;
    const char *op = "";

    if (!n) { printf("NULL"); return; }

    switch (n->type) {
    case NODE_NUM:
        printf("%g", n->value);
        return;
    case NODE_VAR:
        printf("%s", n->name);
        return;
    case NODE_NEG:
        printf("-");
        print_part(n->left, precedence(n->left) <= 3);
        return;
    case NODE_FUNC:
        printf("%s(", func_name(n->func));
        print_expr(n->left);
        printf(")");
        return;
    case NODE_ADD: op = " + "; break;
    case NODE_SUB: op = " - "; break;
    case NODE_MUL: op = "*";   break;
    case NODE_DIV: op = "/";   break;
    case NODE_POW: op = "^";   break;
    }

    p = precedence(n);
    print_part(n->left,
               precedence(n->left) < p ||
               (n->type == NODE_POW && precedence(n->left) == p));
    printf("%s", op);
    print_part(n->right,
               precedence(n->right) < p ||
               (precedence(n->right) == p &&
                (n->type == NODE_SUB || n->type == NODE_DIV)) ||
               (n->right->type == NODE_NUM && n->right->value < 0));
}

void print_tree(const Node *n, int depth)
{
    int i;
    if (!n) return;
    for (i = 0; i < depth; i++) printf("    ");
    switch (n->type) {
    case NODE_NUM:  printf("NUM %g\n",        n->value); break;
    case NODE_VAR:  printf("VAR %s\n",        n->name);  break;
    case NODE_ADD:  printf("+\n");                        break;
    case NODE_SUB:  printf("-\n");                        break;
    case NODE_MUL:  printf("*\n");                        break;
    case NODE_DIV:  printf("/\n");                        break;
    case NODE_POW:  printf("^\n");                        break;
    case NODE_NEG:  printf("NEG\n");                      break;
    case NODE_FUNC: printf("FUNC %s\n", func_name(n->func)); break;
    }
    print_tree(n->left,  depth + 1);
    print_tree(n->right, depth + 1);
}
