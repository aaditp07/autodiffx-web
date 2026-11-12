/*
 * ast.h - shared header for AutoDiffX v2
 *
 * Supports: multi-variable (x, y, z, ...), trig/log/exp functions,
 * partial differentiation, and proper memory management.
 */
#ifndef AST_H
#define AST_H

/* ------------------------------------------------------------------ */
/* Node types                                                           */
/* ------------------------------------------------------------------ */
typedef enum {
    NODE_NUM,   /* constant number, e.g. 3.14    */
    NODE_VAR,   /* variable, e.g. x, y, z        */
    NODE_ADD,   /* left + right                  */
    NODE_SUB,   /* left - right                  */
    NODE_MUL,   /* left * right                  */
    NODE_DIV,   /* left / right                  */
    NODE_POW,   /* left ^ right                  */
    NODE_NEG,   /* -(left)   unary minus         */
    NODE_FUNC   /* f(left)   e.g. sin, cos, ln   */
} NodeType;

/* Function subtypes for NODE_FUNC */
typedef enum {
    FUNC_SIN,
    FUNC_COS,
    FUNC_TAN,
    FUNC_LN,
    FUNC_EXP
} FuncType;

/* ------------------------------------------------------------------ */
/* AST node                                                             */
/* ------------------------------------------------------------------ */
typedef struct Node {
    NodeType type;
    double   value;         /* NODE_NUM  */
    char    *name;          /* NODE_VAR  */
    FuncType func;          /* NODE_FUNC */
    struct Node *left;
    struct Node *right;
} Node;

/* ------------------------------------------------------------------ */
/* Constructors (ast.c)                                                 */
/* ------------------------------------------------------------------ */
Node *new_num (double value);
Node *new_var (char *name);
Node *new_op  (NodeType type, Node *left, Node *right);
Node *new_neg (Node *operand);
Node *new_func(FuncType f,   Node *arg);
Node *copy_node(const Node *n);   /* deep copy */

/* ------------------------------------------------------------------ */
/* Helpers (ast.c)                                                      */
/* ------------------------------------------------------------------ */
int   is_num  (const Node *n, double value);
int   has_var (const Node *n, const char *var);  /* does n contain this var? */
int   is_const_wrt(const Node *n, const char *var); /* no var in n? */
int   eval_const(const Node *n, double *out);    /* evaluate if fully constant */

/* ------------------------------------------------------------------ */
/* Printing (ast.c)                                                     */
/* ------------------------------------------------------------------ */
void  print_expr(const Node *n);
void  print_tree(const Node *n, int depth);

/* ------------------------------------------------------------------ */
/* Memory (ast.c)                                                       */
/* ------------------------------------------------------------------ */
void  free_node(Node *n);

/* ------------------------------------------------------------------ */
/* Semantic analysis (semantic.c)                                       */
/* ------------------------------------------------------------------ */
int   semantic_check(const Node *n);

/* ------------------------------------------------------------------ */
/* Differentiation (diff.c)                                             */
/* ------------------------------------------------------------------ */
Node *differentiate(const Node *n, const char *wrt);

/* ------------------------------------------------------------------ */
/* Optimization / simplification (optimize.c)                           */
/* ------------------------------------------------------------------ */
Node *simplify(Node *n);

/* ------------------------------------------------------------------ */
/* Driver (main.c)                                                      */
/* ------------------------------------------------------------------ */
void  process  (Node *ast, const char *wrt);
void  end_line (void);

/* ------------------------------------------------------------------ */
/* Lexer globals (lexer.l)                                              */
/* ------------------------------------------------------------------ */
extern char token_list[4096];
extern int  lex_errors;

#endif /* AST_H */
