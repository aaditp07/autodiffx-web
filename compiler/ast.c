/*
 * ast.c - Phase 3: build the Abstract Syntax Tree and print it
 */
#include <stdio.h>
#include <stdlib.h>
#include "ast.h"


static Node *new_node(NodeType type)
{
    Node *n = calloc(1, sizeof(Node));   
    n->type = type;
    return n;
}

Node *new_num(double value)
{
    Node *n = new_node(NODE_NUM);
    n->value = (value == 0) ? 0 : value;  
    return n;
}

Node *new_var(char *name)
{
    Node *n = new_node(NODE_VAR);
    n->name = name;
    return n;
}

Node *new_op(NodeType type, Node *left, Node *right)
{
    Node *n = new_node(type);
    n->left = left;
    n->right = right;
    return n;
}

Node *new_neg(Node *operand)
{
    return new_op(NODE_NEG, operand, NULL);
}

/* ---------- small helpers ---------- */

int is_num(Node *n, double value)
{
    return n->type == NODE_NUM && n->value == value;
}

int has_x(Node *n)
{
    if (n == NULL) return 0;
    if (n->type == NODE_VAR) return 1;
    return has_x(n->left) || has_x(n->right);
}


/* Higher number = binds tighter (same order as in parser.y) */
static int precedence(Node *n)
{
    switch (n->type) {
    case NODE_ADD: case NODE_SUB: return 1;
    case NODE_MUL: case NODE_DIV: return 2;
    case NODE_NEG:                return 3;
    case NODE_POW:                return 4;
    case NODE_NUM:                return (n->value < 0) ? 3 : 5;
    default:                      return 5;
    }
}

static void print_part(Node *n, int need_brackets)
{
    if (need_brackets) printf("(");
    print_expr(n);
    if (need_brackets) printf(")");
}

void print_expr(Node *n)
{
    int p;
    const char *op = "";

    switch (n->type) {
    case NODE_NUM: printf("%g", n->value); return;
    case NODE_VAR: printf("%s", n->name);  return;
    case NODE_NEG:
        printf("-");
        print_part(n->left, precedence(n->left) <= 3);
        return;
    case NODE_ADD: op = " + "; break;
    case NODE_SUB: op = " - "; break;
    case NODE_MUL: op = "*";   break;
    case NODE_DIV: op = "/";   break;
    case NODE_POW: op = "^";   break;
    }

    p = precedence(n);

    /* left side: brackets if it binds weaker, e.g. (x + 1)*2, (x^2)^3 */
    print_part(n->left, precedence(n->left) < p ||
                        (n->type == NODE_POW && precedence(n->left) == p));
    printf("%s", op);
    /* right side: brackets if weaker, or equal under - and /, e.g. x - (y + 1),
       or a negative number, e.g. x*(-2) */
    print_part(n->right, precedence(n->right) < p ||
                         (precedence(n->right) == p &&
                          (n->type == NODE_SUB || n->type == NODE_DIV)) ||
                         (n->right->type == NODE_NUM && n->right->value < 0));
}

/* ---------- printing the tree ---------- */

void print_tree(Node *n, int depth)
{
    int i;
    if (n == NULL) return;

    for (i = 0; i < depth; i++) printf("    ");

    switch (n->type) {
    case NODE_NUM: printf("NUM %g\n", n->value); break;
    case NODE_VAR: printf("VAR %s\n", n->name);  break;
    case NODE_ADD: printf("+\n"); break;
    case NODE_SUB: printf("-\n"); break;
    case NODE_MUL: printf("*\n"); break;
    case NODE_DIV: printf("/\n"); break;
    case NODE_POW: printf("^\n"); break;
    case NODE_NEG: printf("NEG (unary -)\n"); break;
    }

    print_tree(n->left, depth + 1);
    print_tree(n->right, depth + 1);
}
