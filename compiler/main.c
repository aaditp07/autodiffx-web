/*
 * main.c - AutoDiffX v2 driver
 *
 * Usage:
 *   ./autodiffx                        (type expressions, Ctrl+D to end)
 *   ./autodiffx < tests/valid.txt      (batch mode)
 *   ./autodiffx -d y                   (differentiate w.r.t. y)
 *   ./autodiffx -d y < tests/multi.txt
 *
 * Changes from v1:
 *  - Accepts -d <var> flag to set the differentiation variable (default: x)
 *  - Shows partial derivative label: ∂f/∂x or df/dx
 *  - Calls free_node() after each line to clean up memory
 *  - Multi-variable support end-to-end
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int  yyparse(void);

/* Global: variable to differentiate with respect to (read by parser/semantic) */
char wrt_var[64] = "x";

/* ------------------------------------------------------------------ */
/* process() — called by the parser for each successfully parsed line  */
/* ------------------------------------------------------------------ */
void process(Node *ast, const char *wrt)
{
    Node *simplified_expr;
    Node *derivative;
    Node *simplified_deriv;

    printf("Tokens     : %s\n", token_list);

    if (lex_errors > 0) {
        printf("Compilation stopped (lexical error).\n");
        free_node(ast);
        return;
    }

    printf("AST        :\n");
    print_tree(ast, 1);

    if (semantic_check(ast) > 0) {
        printf("Compilation stopped (semantic error).\n");
        free_node(ast);
        return;
    }
    printf("Semantic   : OK\n");

    simplified_expr  = simplify(copy_node(ast));
    derivative       = differentiate(ast, wrt);
    simplified_deriv = simplify(derivative);

    printf("Expression : "); print_expr(ast);             printf("\n");
    printf("Simplified : "); print_expr(simplified_expr); printf("\n");

    /* Label partial vs. ordinary derivative */
    if (strcmp(wrt, "x") == 0)
        printf("d/d%s      : ", wrt);
    else
        printf("\u2202/\u2202%s      : ", wrt);  /* ∂ Unicode */
    print_expr(derivative);
    printf("\n");

    printf("Optimized  : "); print_expr(simplified_deriv); printf("\n");

    free_node(ast);
    free_node(simplified_expr);
    free_node(derivative);
    free_node(simplified_deriv);
}

/* ------------------------------------------------------------------ */
/* end_line() — reset per-line state                                   */
/* ------------------------------------------------------------------ */
void end_line(void)
{
    if (token_list[0] != '\0')
        printf("--------------------------------------------------\n");
    token_list[0] = '\0';
    lex_errors    = 0;
}

/* ------------------------------------------------------------------ */
/* main                                                                 */
/* ------------------------------------------------------------------ */
int main(int argc, char *argv[])
{
    int i;

    /* Parse -d <var> flag */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
            strncpy(wrt_var, argv[i + 1], sizeof(wrt_var) - 1);
            wrt_var[sizeof(wrt_var) - 1] = '\0';
            i++;
        } else {
            fprintf(stderr, "Usage: %s [-d variable]\n", argv[0]);
            return 1;
        }
    }

    printf("==================================================\n");
    printf(" AutoDiffX v2 - Symbolic Differentiation Compiler\n");
    printf("==================================================\n");
    printf(" Differentiating with respect to: %s\n", wrt_var);
    printf(" Supported functions: sin cos tan ln exp\n");
    printf(" Supported variables: any single letter (x, y, z, ...)\n");
    printf("--------------------------------------------------\n");
    printf("Enter one expression per line (Ctrl+D to finish):\n");
    printf("--------------------------------------------------\n");

    yyparse();
    return 0;
}
