%option noyywrap
%option yylineno

%{
#include <stdio.h>
#include <io.h>

#include "ast.h"
#include "parser.tab.h"

#ifndef fileno
int _fileno(FILE*);
#define fileno _fileno
#endif

static int hs_lex_error = 0;

void hs_lex_reset_errors(void) { hs_lex_error = 0; }
int hs_lex_had_error(void) { return hs_lex_error; }
%}

WS      [ \t\r\n]+
ID      [a-zA-Z_][a-zA-Z0-9_]*
INT     [0-9]+
FLOAT   [0-9]+\.[0-9]+

%%

{WS}        ;

"rakho"     { return RAKHO; }
"me"        { return ME; }
"likho"     { return LIKHO; }
"agar"      { return AGAR; }
"to"        { return TO; }
"warna"     { return WARNA; }
"jabtak"    { return JABTAK; }
"jab"       { return JAB; }
"anya"      { return ANYA; }
"shuru"     { return SHURU; }
"khatam"    { return KHATAM; }
"banao"     { return BANAO; }
"wapas"     { return WAPAS; }

"|>"        { return PIPE; }
">="        { return GTE; }
"<="        { return LTE; }
"=="        { return EQ; }
"!="        { return NEQ; }
">"         { return GT; }
"<"         { return LT; }

"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return MUL; }
"/"         { return DIV; }

"("         { return LPAREN; }
")"         { return RPAREN; }
","         { return COMMA; }

[0-9]+\.[0-9]+\.[0-9]+ {
    fprintf(stderr, "Error [lexical] line %d: invalid number '%s'\n", yylineno, yytext);
    hs_lex_error = 1;
    return 0;
}

{FLOAT}     {
    yylval.number = atof(yytext);
    return NUMBER;
}

{INT}       {
    yylval.number = atof(yytext);
    return NUMBER;
}

{ID}        {
    yylval.text = hs_strdup(yytext);
    return IDENT;
}

.           {
    fprintf(stderr, "Error [lexical] line %d: illegal character '%s'\n", yylineno, yytext);
    hs_lex_error = 1;
    return 0;
}

%%
