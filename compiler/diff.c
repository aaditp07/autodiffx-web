/*
 * diff.c - Phase 5: Symbolic Differentiation for AutoDiffX v2
 *
 * Changes from v1:
 *  - All rules now take `wrt` (the variable to differentiate w.r.t.)
 *  - Multi-variable: d(y)/dx = 0, d(x)/dx = 1
 *  - Trig / log / exp chain rules added
 *  - copy_node() used to avoid pointer aliasing in product/quotient rules
 *
 * Differentiation rules implemented:
 *  Constant    : d(c)/d(v) = 0
 *  Variable    : d(v)/d(v) = 1,  d(u)/d(v) = 0  (u ≠ v)
 *  Sum         : d(u+w)/dv = u' + w'
 *  Difference  : d(u-w)/dv = u' - w'
 *  Product     : d(u·w)/dv = u'·w + u·w'
 *  Quotient    : d(u/w)/dv = (u'·w - u·w') / w²
 *  Power       : d(u^n)/dv = n·u^(n-1)·u'    (n constant w.r.t. v)
 *  Neg         : d(-u)/dv  = -u'
 *  sin(u)      : cos(u)·u'
 *  cos(u)      : -sin(u)·u'
 *  tan(u)      : (1/cos²(u))·u'
 *  ln(u)       : u'/u
 *  exp(u)      : exp(u)·u'
 */
#include "ast.h"

Node *differentiate(const Node *n, const char *wrt)
{
    const Node *u, *v;
    Node *du, *dv;

    if (!n) return new_num(0);

    u = n->left;
    v = n->right;

    switch (n->type) {

    /* ---- Constants and variables ---- */
    case NODE_NUM:
        return new_num(0);

    case NODE_VAR:
        return new_num(strcmp(n->name, wrt) == 0 ? 1 : 0);

    /* ---- Arithmetic ---- */
    case NODE_ADD:
        return new_op(NODE_ADD,
                      differentiate(u, wrt),
                      differentiate(v, wrt));

    case NODE_SUB:
        return new_op(NODE_SUB,
                      differentiate(u, wrt),
                      differentiate(v, wrt));

    case NODE_NEG:
        return new_neg(differentiate(u, wrt));

    case NODE_MUL:
        /* Product rule: u'v + uv' */
        du = differentiate(u, wrt);
        dv = differentiate(v, wrt);
        return new_op(NODE_ADD,
                      new_op(NODE_MUL, du, copy_node(v)),
                      new_op(NODE_MUL, copy_node(u), dv));

    case NODE_DIV:
        /* Quotient rule: (u'v - uv') / v² */
        du = differentiate(u, wrt);
        dv = differentiate(v, wrt);
        return new_op(NODE_DIV,
                      new_op(NODE_SUB,
                             new_op(NODE_MUL, du, copy_node(v)),
                             new_op(NODE_MUL, copy_node(u), dv)),
                      new_op(NODE_POW, copy_node(v), new_num(2)));

    case NODE_POW:
        /* Power rule: n·u^(n-1)·u'  (exponent must be constant w.r.t. wrt) */
        if (is_const_wrt(u, wrt))
            return new_num(0);  /* constant^constant */
        du = differentiate(u, wrt);
        return new_op(NODE_MUL,
                      new_op(NODE_MUL,
                             copy_node(v),
                             new_op(NODE_POW,
                                    copy_node(u),
                                    new_op(NODE_SUB, copy_node(v), new_num(1)))),
                      du);

    /* ---- Transcendental functions (chain rule applied) ---- */
    case NODE_FUNC: {
        du = differentiate(u, wrt);   /* inner derivative */

        switch (n->func) {

        case FUNC_SIN:
            /* d/dv[sin(u)] = cos(u)·u' */
            return new_op(NODE_MUL, new_func(FUNC_COS, copy_node(u)), du);

        case FUNC_COS:
            /* d/dv[cos(u)] = -sin(u)·u' */
            return new_op(NODE_MUL,
                          new_neg(new_func(FUNC_SIN, copy_node(u))),
                          du);

        case FUNC_TAN:
            /* d/dv[tan(u)] = u' / cos²(u) */
            return new_op(NODE_DIV,
                          du,
                          new_op(NODE_POW,
                                 new_func(FUNC_COS, copy_node(u)),
                                 new_num(2)));

        case FUNC_LN:
            /* d/dv[ln(u)] = u'/u */
            return new_op(NODE_DIV, du, copy_node(u));

        case FUNC_EXP:
            /* d/dv[exp(u)] = exp(u)·u' */
            return new_op(NODE_MUL, new_func(FUNC_EXP, copy_node(u)), du);
        }
        break;
    }

    } /* switch */

    return new_num(0);
}
