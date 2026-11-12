/*
 * optimize.c - Phase 6: Optimization (simplification)
 */
#include "ast.h"

static int power(double base, double exp, double *result)
{
    int i;
    if (exp < 0 || exp > 1000 || exp != (int)exp) return 0;  
    *result = 1;
    for (i = 0; i < (int)exp; i++) *result *= base;
    return 1;
}

Node *simplify(Node *n)
{
    Node *l, *r;
    double v;

    if (n->type == NODE_NUM || n->type == NODE_VAR)
        return n;

    /* unary minus */
    if (n->type == NODE_NEG) {
        l = simplify(n->left);
        if (l->type == NODE_NUM)                        /* -(4) -> -4 */
            return new_num(-l->value);
        if (l->type == NODE_MUL && l->left->type == NODE_NUM)   /* -(2*x) -> -2*x */
            return new_op(NODE_MUL, new_num(-l->left->value), l->right);
        return new_neg(l);
    }

    l = simplify(n->left);
    r = simplify(n->right);

    /* ---- constant folding: both sides are numbers ---- */
    if (l->type == NODE_NUM && r->type == NODE_NUM) {
        switch (n->type) {
        case NODE_ADD: return new_num(l->value + r->value);
        case NODE_SUB: return new_num(l->value - r->value);
        case NODE_MUL: return new_num(l->value * r->value);
        case NODE_DIV: if (r->value != 0) return new_num(l->value / r->value); break;
        case NODE_POW: if (power(l->value, r->value, &v)) return new_num(v); break;
        default: break;
        }
    }

    switch (n->type) {
    case NODE_ADD:
        if (is_num(l, 0)) return r;                     
        if (is_num(r, 0)) return l;                     
        break;

    case NODE_SUB:
        if (is_num(r, 0)) return l;                     
        if (is_num(l, 0)) return simplify(new_neg(r));  
        break;

    case NODE_MUL:
        if (is_num(l, 0) || is_num(r, 0)) return new_num(0);   
        if (is_num(l, 1)) return r;                    
        if (is_num(r, 1)) return l;                     
        if (r->type == NODE_NUM && l->type != NODE_NUM) 
            return simplify(new_op(NODE_MUL, r, l));
        if (l->type == NODE_NUM && r->type == NODE_MUL && r->left->type == NODE_NUM)
            return simplify(new_op(NODE_MUL,            
                                   new_num(l->value * r->left->value), r->right));
        break;

    case NODE_DIV:
        if (is_num(l, 0)) return new_num(0);            
        if (is_num(r, 1)) return l;                    
        break;

    case NODE_POW:
        if (is_num(r, 0)) return new_num(1);            
        if (is_num(r, 1)) return l;                     
        break;

    default:
        break;
    }

    return new_op(n->type, l, r);
}
