%{
/*
 * lexer.l - Phase 1: Lexical Analysis for AutoDiffX v2
 *
 * New tokens in v2:
 *  - FUNC  : sin, cos, tan, ln, exp
 *  - ID    : any identifier (single or multi-char) used as variable
 *  - Multi-char variables are allowed; semantic.c decides validity
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

char token_list[4096];
int  lex_errors = 0;

static void add_token(const char *name)
{
    char buf[256];
    if (strcmp(name, "NUMBER") == 0 || strcmp(name, "ID") == 0 ||
        strcmp(name, "FUNC") == 0)
        snprintf(buf, sizeof buf, "%s(%s) ", name, yytext);
    else
        snprintf(buf, sizeof buf, "%s ", name);
    if (strlen(token_list) + strlen(buf) < sizeof token_list)
        strcat(token_list, buf);
}
%}

%option noyywrap nounput noinput

%%

[ \t\r]+              { /* skip whitespace */ }
"#".*                 { /* skip line comments */ }

[0-9]+(\.[0-9]+)?     {
                        add_token("NUMBER");
                        yylval.num = atof(yytext);
                        return NUMBER;
                      }

"sin"                 { add_token("FUNC"); yylval.func = FUNC_SIN; return FUNC; }
"cos"                 { add_token("FUNC"); yylval.func = FUNC_COS; return FUNC; }
"tan"                 { add_token("FUNC"); yylval.func = FUNC_TAN; return FUNC; }
"ln"                  { add_token("FUNC"); yylval.func = FUNC_LN;  return FUNC; }
"exp"                 { add_token("FUNC"); yylval.func = FUNC_EXP; return FUNC; }

[a-zA-Z][a-zA-Z0-9_]* {
                        add_token("ID");
                        yylval.name = strdup(yytext);
                        return ID;
                      }

"+"                   { add_token("PLUS");   return '+'; }
"-"                   { add_token("MINUS");  return '-'; }
"*"                   { add_token("MUL");    return '*'; }
"/"                   { add_token("DIV");    return '/'; }
"^"                   { add_token("POW");    return '^'; }
"("                   { add_token("LPAREN"); return '('; }
")"                   { add_token("RPAREN"); return ')'; }
","                   { add_token("COMMA");  return ','; }

\n                    { return '\n'; }

<<EOF>>               {
                        static int sent = 0;
                        if (!sent) { sent = 1; return '\n'; }
                        return 0;
                      }

.                     {
                        printf("Lexical error: unexpected character '%s'\n", yytext);
                        lex_errors++;
                      }

%%
