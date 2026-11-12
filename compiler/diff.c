/*
 * diff.c - Phase 5: Symbolic Differentiation
 */
#include "ast.h"

Node *differentiate(Node *n)
{
    Node *u = n->left;
    Node *v = n->right;

    switch (n->type) {
    case NODE_NUM:
        return new_num(0);

    case NODE_VAR:
        return new_num(1);

    case NODE_ADD:
        return new_op(NODE_ADD, differentiate(u), differentiate(v));

    case NODE_SUB:
        return new_op(NODE_SUB, differentiate(u), differentiate(v));

    case NODE_NEG:
        return new_neg(differentiate(u));

    case NODE_MUL:
        return new_op(NODE_ADD,
                      new_op(NODE_MUL, differentiate(u), v),
                      new_op(NODE_MUL, u, differentiate(v)));

    case NODE_DIV:
        return new_op(NODE_DIV,
                      new_op(NODE_SUB,
                             new_op(NODE_MUL, differentiate(u), v),
                             new_op(NODE_MUL, u, differentiate(v))),
                      new_op(NODE_POW, v, new_num(2)));

    case NODE_POW:
        if (!has_x(u))              
            return new_num(0);
        return new_op(NODE_MUL,
                      new_op(NODE_MUL,
                             v,
                             new_op(NODE_POW, u, new_op(NODE_SUB, v, new_num(1)))),
                      differentiate(u));
    }
    return new_num(0);
}
