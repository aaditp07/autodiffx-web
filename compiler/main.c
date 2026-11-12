/*
 * main.c - AutoDiffX driver
 *
 * Phase 1 (lexer.l) and Phase 2 (parser.y) run inside yyparse().
 * For every correct line the parser calls process(), which runs
 * the remaining phases and prints the results.
 *
 * Run:  ./autodiffx              (type expressions, Ctrl+D to finish)
 *       ./autodiffx < tests/valid.txt
 */
#include <stdio.h>
#include "ast.h"

int yyparse(void);

void process(Node *ast)
{
    Node *derivative;

    printf("Tokens     : %s\n", token_list);
    if (lex_errors > 0) {
        printf("Compilation stopped (lexical error).\n");
        return;
    }

    printf("AST        :\n");
    print_tree(ast, 1);

    if (semantic_check(ast) > 0) {
        printf("Compilation stopped (semantic error).\n");
        return;
    }
    printf("Semantic   : OK\n");

    derivative = differentiate(ast);

    printf("Expression : "); print_expr(ast);                  printf("\n");
    printf("Simplified : "); print_expr(simplify(ast));        printf("\n");
    printf("Derivative : "); print_expr(derivative);           printf("\n");
    printf("Optimized  : "); print_expr(simplify(derivative)); printf("\n");
}

void end_line(void)
{
    if (token_list[0] != '\0')
        printf("--------------------------------------------------\n");
    token_list[0] = '\0';
    lex_errors = 0;
}

int main(void)
{
    printf("==================================================\n");
    printf(" AutoDiffX - Symbolic Differentiation Mini-Compiler\n");
    printf("==================================================\n");
    printf("Enter one expression in x per line:\n");
    printf("--------------------------------------------------\n");
    yyparse();
    return 0;
}
