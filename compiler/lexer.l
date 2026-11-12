%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

char token_list[4096];   
int  lex_errors = 0;     

static void add_token(const char *name)
{
    char buf[128];
    if (strcmp(name, "NUMBER") == 0 || strcmp(name, "ID") == 0)
        snprintf(buf, sizeof buf, "%s(%s) ", name, yytext);
    else
        snprintf(buf, sizeof buf, "%s ", name);
    if (strlen(token_list) + strlen(buf) < sizeof token_list)
        strcat(token_list, buf);
}
%}

%option noyywrap nounput noinput

%%

[ \t\r]+              { /* skip spaces */ }
"#".*                 { /* skip comments */ }

[0-9]+(\.[0-9]+)?     { add_token("NUMBER"); yylval.num = atof(yytext); return NUMBER; }
[a-zA-Z]+             { add_token("ID");     yylval.name = strdup(yytext); return ID; }

"+"                   { add_token("PLUS");   return '+'; }
"-"                   { add_token("MINUS");  return '-'; }
"*"                   { add_token("MUL");    return '*'; }
"/"                   { add_token("DIV");    return '/'; }
"^"                   { add_token("POW");    return '^'; }
"("                   { add_token("LPAREN"); return '('; }
")"                   { add_token("RPAREN"); return ')'; }

\n                    { return '\n'; }

<<EOF>>               { /* send one final newline so the last line is
                           processed even if the file does not end with one */
                        static int sent = 0;
                        if (!sent) { sent = 1; return '\n'; }
                        return 0; }

.                     { printf("Lexical error: unexpected character '%s'\n", yytext);
                        lex_errors++; }

%%
