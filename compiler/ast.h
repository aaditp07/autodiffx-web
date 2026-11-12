/*
 * ast.h - shared header for AutoDiffX
 *
 * An expression is stored as a tree (AST). Example for 2*x + 1:
 *
 *          +
 *        /   \
 *       *     1
 *      / \
 *     2   x
 */
#ifndef AST_H
#define AST_H

/* Kinds of nodes in the tree */
typedef enum {
    NODE_NUM,   /* number, e.g. 3     */
    NODE_VAR,   /* variable, e.g. x   */
    NODE_ADD,   /* left + right       */
    NODE_SUB,   /* left - right       */
    NODE_MUL,   /* left * right       */
    NODE_DIV,   /* left / right       */
    NODE_POW,   /* left ^ right       */
    NODE_NEG    /* -left (unary minus)*/
} NodeType;

typedef struct Node {
    NodeType type;
    double value;          /* used when type == NODE_NUM */
    char *name;            /* used when type == NODE_VAR */
    struct Node *left;
    struct Node *right;
} Node;

/* ast.c : create and print trees */
Node *new_num(double value);
Node *new_var(char *name);
Node *new_op(NodeType type, Node *left, Node *right);
Node *new_neg(Node *operand);
int   is_num(Node *n, double value);   /* is n exactly this number? */
int   has_x(Node *n);                  /* does n contain a variable? */
void  print_expr(Node *n);             /* print as normal math text  */
void  print_tree(Node *n, int depth);  /* print the AST, indented    */

/* semantic.c : Phase 4 */
int   semantic_check(Node *n);         /* returns number of errors   */

/* diff.c : Phase 5 */
Node *differentiate(Node *n);

/* optimize.c : Phase 6 */
Node *simplify(Node *n);

/* main.c : runs all phases for one parsed line */
void  process(Node *ast);
void  end_line(void);

/* lexer.l : shared with main.c */
extern char token_list[];
extern int  lex_errors;

#endif
