/*
 * parser.y - Phase 2: Syntax Analysis
 */
%{
#include <stdio.h>
#include "ast.h"

int yylex(void);
void yyerror(const char *msg);
extern char *yytext;
%}

/* values that tokens and rules can carry */
%union {
    double num;
    char  *name;
    Node  *node;
}

%token <num>  NUMBER
%token <name> ID
%type  <node> expr

/* precedence: lowest first */
%left  '+' '-'
%left  '*' '/'
%right UMINUS          
%right '^'            

%%

input : /* empty */
      | input line
      ;

line  : '\n'                { end_line(); }
      | expr '\n'           { process($1); end_line(); }
      | error '\n'          { printf("Tokens     : %s\n", token_list);
                              printf("Compilation stopped (%s error).\n",
                                     lex_errors > 0 ? "lexical" : "syntax");
                              yyerrok; end_line(); }
      ;

expr  : expr '+' expr          { $$ = new_op(NODE_ADD, $1, $3); }
      | expr '-' expr          { $$ = new_op(NODE_SUB, $1, $3); }
      | expr '*' expr          { $$ = new_op(NODE_MUL, $1, $3); }
      | expr '/' expr          { $$ = new_op(NODE_DIV, $1, $3); }
      | expr '^' expr          { $$ = new_op(NODE_POW, $1, $3); }
      | '-' expr %prec UMINUS  { $$ = new_neg($2); }
      | '(' expr ')'           { $$ = $2; }
      | NUMBER                 { $$ = new_num($1); }
      | ID                     { $$ = new_var($1); }
      ;

%%

void yyerror(const char *msg)
{
    if (lex_errors > 0) return;   
    if (yytext[0] == '\n')
        printf("Syntax error: %s at end of line\n", msg);
    else
        printf("Syntax error: %s near '%s'\n", msg, yytext);
}
