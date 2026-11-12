/*
 * optimize.c - Phase 6: Optimization and Simplification for AutoDiffX v2
 *
 * Changes from v1:
 *  - NODE_FUNC: constant folding for trig/log/exp with numeric args
 *  - Like-term collection: combines coefficient × x^n terms
 *    e.g.  3*x^2 + 4*x^2  →  7*x^2
 *          2*x + x         →  3*x
 *  - Multi-variable aware (treats other vars as opaque)
 *  - Constant sub-expression evaluation via eval_const()
 *  - Basic trig identity: sin²(u) + cos²(u) = 1 (single-variable)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ast.h"

/* ================================================================== */
/* Forward declaration                                                  */
/* ================================================================== */
static Node *simplify_once(Node *n);

/* ================================================================== */
/* Like-term collection helpers                                         */
/* ================================================================== */

/*
 * A "power term" has the shape  coeff * base^exp  where:
 *   coeff is a double constant
 *   base  is any sub-expression (may be a variable, function, etc.)
 *   exp   is a constant double  (1 by default)
 */
typedef struct {
    double  coeff;
    Node   *base;    /* NOT owned — just a reference */
    double  exponent;
} PowerTerm;

/* Try to decompose n into a PowerTerm.  Returns 1 on success. */
static int decompose(Node *n, PowerTerm *out)
{
    double cv;
    /* Just a constant */
    if (n->type == NODE_NUM) {
        out->coeff    = n->value;
        out->base     = NULL;
        out->exponent = 0;
        return 1;
    }
    /* Pure variable or function: coeff=1, exp=1 */
    if (n->type == NODE_VAR || n->type == NODE_FUNC) {
        out->coeff    = 1;
        out->base     = n;
        out->exponent = 1;
        return 1;
    }
    /* c * base  or  base * c */
    if (n->type == NODE_MUL) {
        if (n->left->type == NODE_NUM) {
            PowerTerm sub;
            if (decompose(n->right, &sub) && sub.coeff == 1) {
                out->coeff    = n->left->value * sub.coeff;
                out->base     = sub.base;
                out->exponent = sub.exponent;
                return 1;
            }
        }
        if (n->right->type == NODE_NUM) {
            PowerTerm sub;
            if (decompose(n->left, &sub) && sub.coeff == 1) {
                out->coeff    = n->right->value * sub.coeff;
                out->base     = sub.base;
                out->exponent = sub.exponent;
                return 1;
            }
        }
    }
    /* base ^ c */
    if (n->type == NODE_POW && eval_const(n->right, &cv)) {
        out->coeff    = 1;
        out->base     = n->left;
        out->exponent = cv;
        return 1;
    }
    /* c * base^exp */
    if (n->type == NODE_MUL && n->left->type == NODE_NUM
        && n->right->type == NODE_POW
        && eval_const(n->right->right, &cv)) {
        out->coeff    = n->left->value;
        out->base     = n->right->left;
        out->exponent = cv;
        return 1;
    }
    return 0;
}

/*
 * Check if two base expressions are structurally identical
 * (syntactic equality, sufficient for like-term detection).
 */
static int expr_equal(const Node *a, const Node *b)
{
    if (!a && !b) return 1;
    if (!a || !b) return 0;
    if (a->type != b->type) return 0;
    switch (a->type) {
    case NODE_NUM: return a->value == b->value;
    case NODE_VAR: return strcmp(a->name, b->name) == 0;
    case NODE_FUNC:
        return a->func == b->func && expr_equal(a->left, b->left);
    default:
        return expr_equal(a->left, b->left) && expr_equal(a->right, b->right);
    }
}

/* Build  coeff * base^exp  node */
static Node *build_power_term(double coeff, Node *base, double exp)
{
    Node *term;
    if (!base) return new_num(coeff);

    if (exp == 1)
        term = copy_node(base);
    else if (exp == 0)
        return new_num(coeff);
    else
        term = new_op(NODE_POW, copy_node(base), new_num(exp));

    if (coeff == 1)  return term;
    if (coeff == -1) return new_neg(term);
    return new_op(NODE_MUL, new_num(coeff), term);
}

/*
 * Collect like terms in a sum/difference chain.
 * We flatten the chain into a list of PowerTerms, combine duplicates,
 * then rebuild the tree.
 *
 * Returns NULL if no like terms were found (no change).
 */
#define MAX_TERMS 64

static Node *collect_like_terms(Node *n)
{
    PowerTerm terms[MAX_TERMS];
    int       count = 0;
    int       changed = 0;

    /* ---- Flatten ADD/SUB chain ---- */
    /* We use an iterative traversal on the left spine */
    typedef struct { Node *node; int sign; } Frame;
    Frame stack[MAX_TERMS * 2];
    int   sp = 0;
    stack[sp].node = n; stack[sp].sign = 1; sp++;

    while (sp > 0) {
        Frame f = stack[--sp];
        Node *cur  = f.node;
        int   sign = f.sign;

        if (cur->type == NODE_ADD || cur->type == NODE_SUB) {
            /* right child with adjusted sign */
            int rsign = (cur->type == NODE_SUB) ? -sign : sign;
            stack[sp].node = cur->right; stack[sp].sign = rsign; sp++;
            stack[sp].node = cur->left;  stack[sp].sign = sign;  sp++;
        } else {
            PowerTerm pt;
            Node *actual = cur;

            /* Handle negation */
            int s = sign;
            if (actual->type == NODE_NEG) {
                s = -s;
                actual = actual->left;
            }

            if (count < MAX_TERMS && decompose(actual, &pt)) {
                /* Look for a matching like term */
                int found = 0, i;
                for (i = 0; i < count; i++) {
                    if (terms[i].exponent == pt.exponent &&
                        expr_equal(terms[i].base, pt.base)) {
                        terms[i].coeff += s * pt.coeff;
                        found = 1;
                        changed = 1;
                        break;
                    }
                }
                if (!found) {
                    pt.coeff *= s;
                    terms[count++] = pt;
                }
            } else if (count < MAX_TERMS) {
                /* Can't decompose — treat as opaque term with coeff sign */
                terms[count].coeff    = (double)s;
                terms[count].base     = cur;   /* keep as-is */
                terms[count].exponent = -999;  /* sentinel: opaque */
                count++;
            }
        }
    }

    if (!changed) return NULL;

    /* ---- Rebuild the sum from terms ---- */
    Node *result = NULL;
    int i;
    for (i = 0; i < count; i++) {
        Node *term;
        if (terms[i].coeff == 0) continue;

        if (terms[i].exponent == -999) {
            /* opaque */
            term = copy_node(terms[i].base);
            if (terms[i].coeff < 0)
                term = new_neg(term);
        } else {
            term = build_power_term(fabs(terms[i].coeff),
                                    terms[i].base,
                                    terms[i].exponent);
        }

        if (!result) {
            result = (terms[i].coeff < 0 && terms[i].exponent != -999)
                     ? new_neg(term) : term;
        } else {
            if (terms[i].coeff < 0 && terms[i].exponent != -999)
                result = new_op(NODE_SUB, result, term);
            else
                result = new_op(NODE_ADD, result, term);
        }
    }

    return result ? result : new_num(0);
}

/* ================================================================== */
/* Core simplification                                                  */
/* ================================================================== */

static Node *simplify_once(Node *n)
{
    Node   *l, *r;
    double  v, cv;

    if (!n) return new_num(0);

    /* Leaves */
    if (n->type == NODE_NUM || n->type == NODE_VAR)
        return n;

    /* Functions: constant-fold or simplify argument */
    if (n->type == NODE_FUNC) {
        Node *arg = simplify(n->left);
        if (eval_const(arg, &v)) {
            double res;
            switch (n->func) {
            case FUNC_SIN: res = sin(v);  free_node(arg); return new_num(res);
            case FUNC_COS: res = cos(v);  free_node(arg); return new_num(res);
            case FUNC_TAN: res = tan(v);  free_node(arg); return new_num(res);
            case FUNC_LN:  if (v > 0) { res = log(v); free_node(arg); return new_num(res); } break;
            case FUNC_EXP: res = exp(v);  free_node(arg); return new_num(res);
            }
        }
        n->left = arg;
        return n;
    }

    /* Unary minus */
    if (n->type == NODE_NEG) {
        l = simplify(n->left);
        if (l->type == NODE_NUM) return new_num(-l->value);
        if (l->type == NODE_NEG) return l->left;   /* -(-x) = x */
        if (l->type == NODE_MUL && l->left->type == NODE_NUM)
            return new_op(NODE_MUL, new_num(-l->left->value), l->right);
        return new_neg(l);
    }

    /* Binary operators: recurse first */
    l = simplify(n->left);
    r = simplify(n->right);

    /* ---- Full constant folding (both sides evaluate to numbers) ---- */
    if (eval_const(l, &v) && eval_const(r, &cv)) {
        double res;
        switch (n->type) {
        case NODE_ADD: res = v + cv; goto ret_const;
        case NODE_SUB: res = v - cv; goto ret_const;
        case NODE_MUL: res = v * cv; goto ret_const;
        case NODE_DIV:
            if (cv != 0.0) { res = v / cv; goto ret_const; }
            break;
        case NODE_POW:
            res = pow(v, cv);
            if (!isinf(res) && !isnan(res)) goto ret_const;
            break;
        default: break;
        }
        goto combine;
    ret_const:
        free_node(l); free_node(r);
        return new_num(res);
    }

combine:
    switch (n->type) {

    case NODE_ADD:
        if (is_num(l, 0)) { free_node(l); return r; }
        if (is_num(r, 0)) { free_node(r); return l; }
        break;

    case NODE_SUB:
        if (is_num(r, 0)) { free_node(r); return l; }
        if (is_num(l, 0)) { free_node(l); return simplify(new_neg(r)); }
        /* x - x = 0 */
        if (expr_equal(l, r)) { free_node(l); free_node(r); return new_num(0); }
        break;

    case NODE_MUL:
        if (is_num(l, 0) || is_num(r, 0)) {
            free_node(l); free_node(r); return new_num(0);
        }
        if (is_num(l, 1)) { free_node(l); return r; }
        if (is_num(r, 1)) { free_node(r); return l; }
        if (is_num(l, -1)) { free_node(l); return simplify(new_neg(r)); }
        if (is_num(r, -1)) { free_node(r); return simplify(new_neg(l)); }
        /* Normalize: constant always on the left */
        if (r->type == NODE_NUM && l->type != NODE_NUM) {
            Node *tmp = l; l = r; r = tmp;
        }
        /* Merge constants: 2*(3*x) -> 6*x */
        if (l->type == NODE_NUM && r->type == NODE_MUL && r->left->type == NODE_NUM)
            return simplify(new_op(NODE_MUL,
                                   new_num(l->value * r->left->value), r->right));
        break;

    case NODE_DIV:
        if (is_num(l, 0)) { free_node(r); return new_num(0); }
        if (is_num(r, 1)) { free_node(r); return l; }
        /* x / x = 1 */
        if (expr_equal(l, r)) { free_node(l); free_node(r); return new_num(1); }
        break;

    case NODE_POW:
        if (is_num(r, 0)) { free_node(l); free_node(r); return new_num(1); }
        if (is_num(r, 1)) { free_node(r); return l; }
        if (is_num(l, 0)) { free_node(r); return new_num(0); }
        if (is_num(l, 1)) { free_node(r); return new_num(1); }
        /* (x^a)^b = x^(a*b) if a,b are constants */
        if (l->type == NODE_POW && eval_const(l->right, &v) && eval_const(r, &cv))
            return simplify(new_op(NODE_POW, l->left, new_num(v * cv)));
        break;

    default:
        break;
    }

    return new_op(n->type, l, r);
}

/* ================================================================== */
/* Public entry: multi-pass simplification until fixed point           */
/* ================================================================== */

Node *simplify(Node *n)
{
    Node *cur = n;
    Node *next;
    int   pass, MAX_PASS = 8;

    for (pass = 0; pass < MAX_PASS; pass++) {
        /* 1. Structural simplification */
        next = simplify_once(cur);

        /* 2. Like-term collection on ADD/SUB roots */
        if (next->type == NODE_ADD || next->type == NODE_SUB) {
            Node *collected = collect_like_terms(next);
            if (collected) {
                free_node(next);
                next = collected;
                /* One more structural pass after collection */
                Node *again = simplify_once(next);
                free_node(next);
                next = again;
            }
        }

        /* Fixed-point check (structural equality) */
        if (expr_equal(cur, next)) {
            if (cur != n) free_node(cur);
            return next;
        }
        if (cur != n) free_node(cur);
        cur = next;
    }
    return cur;
}
