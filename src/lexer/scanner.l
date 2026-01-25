%option noyywrap
%option yylineno

%{

#include <stdio.h>

/* prints the tokens */
static void tok(const char* name,const char* lexeme){
    if(lexeme) printf("%s(%s)\n",name,lexeme);
    else printf("%s\n",name);
}

%}

/* macros */
WS          [ \t\r]+
ID          [a-zA-Z_][a-zA-Z0-9_]*
INT         [0-9]+
FLOAT       [0-9]+\.[0-9]+

%%

{WS}        ;

\n+         { tok("TOK_NEWLINE","\\n"); }

"rakho"     { tok("TOK_RAKHO",yytext); }
"me"        { tok("TOK_ME",yytext); }
"likho"     { tok("TOK_LIKHO",yytext); }
"agar"      { tok("TOK_AGAR",yytext); }
"to"        { tok("TOK_TO",yytext); }
"warna"     { tok("TOK_WARNA",yytext); }
"jabtak"    { tok("TOK_JABTAK",yytext); }
"shuru"     { tok("TOK_SHURU",yytext); }
"khatam"    { tok("TOK_KHATAM",yytext); }
"banao"     { tok("TOK_BANAO",yytext); }
"wapas"     { tok("TOK_WAPAS",yytext); }

">="        { tok("TOK_GTE",yytext); }
"<="        { tok("TOK_LTE",yytext); }
"=="        { tok("TOK_EQ",yytext); }
"!="        { tok("TOK_NEQ",yytext); }
">"         { tok("TOK_GT",yytext); }
"<"         { tok("TOK_LT",yytext); }
"="         { tok("TOK_ASSIGN",yytext); }

"+"         { tok("TOK_PLUS",yytext); }
"-"         { tok("TOK_MINUS",yytext); }
"*"         { tok("TOK_MUL",yytext); }
"/"         { tok("TOK_DIV",yytext); }

"("         { tok("TOK_LPAREN",yytext); }
")"         { tok("TOK_RPAREN",yytext); }
","         { tok("TOK_COMMA",yytext); }

[0-9]+\.[0-9]+\.[0-9]+ {
 fprintf(stderr,"Lexical Error: invalid number '%s' at line %d\n",yytext,yylineno);
 return 1;
}

{FLOAT}     { tok("TOK_FLOAT",yytext); }
{INT}       { tok("TOK_INT",yytext); }

{ID}        { tok("TOK_ID",yytext); }

.           {
 fprintf(stderr,"Lexical Error: '%s' at line %d\n",yytext,yylineno);
 return 1;
}

%%
