%{
/*
 * parser.y - Phase 2: Syntax Analysis for AutoDiffX v2
 *
 * Grammar extensions from v1:
 *  - Function calls: sin(expr), cos(expr), tan(expr), ln(expr), exp(expr)
 *  - Multi-variable expressions: x^2 + y^2 + z
 *  - Partial derivative target passed via global `wrt_var`
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int  yylex(void);
void yyerror(const char *msg);

/* Variable to differentiate with respect to (set by main) */
extern char wrt_var[64];

%}

%union {
    double   num;
    char    *name;
    FuncType func;
    Node    *node;
}

%token <num>  NUMBER
%token <name> ID
%token <func> FUNC

%type  <node> expr

/* Operator precedence (lowest → highest) */
%right  UMINUS
%left   '+' '-'
%left   '*' '/'
%right  '^'

%%

program
    : /* empty */
    | program line
    ;

line
    : expr '\n'      { process($1, wrt_var); end_line(); }
    | '\n'           { end_line(); }
    | error '\n'     { yyerrok; end_line(); }
    ;

expr
    : expr '+' expr         { $$ = new_op(NODE_ADD, $1, $3); }
    | expr '-' expr         { $$ = new_op(NODE_SUB, $1, $3); }
    | expr '*' expr         { $$ = new_op(NODE_MUL, $1, $3); }
    | expr '/' expr         { $$ = new_op(NODE_DIV, $1, $3); }
    | expr '^' expr         { $$ = new_op(NODE_POW, $1, $3); }
    | '-' expr %prec UMINUS { $$ = new_neg($2); }
    | '(' expr ')'          { $$ = $2; }
    | FUNC '(' expr ')'     { $$ = new_func($1, $3); }
    | NUMBER                { $$ = new_num($1); }
    | ID                    { $$ = new_var($1); }
    ;

%%

void yyerror(const char *msg)
{
    printf("Syntax error: %s\n", msg);
}
