%{
#include <stdio.h>
#include <stdlib.h>

#include "ast.h"

int yylex(void);
void yyrestart(FILE* input_file);
extern int yylineno;

static Program* g_program = NULL;
static int g_parse_error = 0;

static StmtVec hs_empty_stmt_vec(void) {
    StmtVec v;
    v.len = 0;
    v.cap = 0;
    v.items = NULL;
    return v;
}

static StrVec hs_empty_str_vec(void) {
    StrVec v;
    v.len = 0;
    v.cap = 0;
    v.items = NULL;
    return v;
}

static ExprVec hs_empty_expr_vec(void) {
    ExprVec v;
    v.len = 0;
    v.cap = 0;
    v.items = NULL;
    return v;
}

void yyerror(const char* s) {
    fprintf(stderr, "Error [syntax] line %d: %s\n", yylineno, s);
    g_parse_error = 1;
}
%}

%union {
    double number;
    char* text;
    Expr* expr;
    Stmt* stmt;
    StmtVec stmts;
    StrVec strs;
    ExprVec exprs;
}

%token RAKHO ME LIKHO AGAR TO WARNA JABTAK JAB ANYA SHURU KHATAM BANAO WAPAS
%token GTE LTE EQ NEQ GT LT
%token PLUS MINUS MUL DIV PIPE
%token LPAREN RPAREN COMMA
%token <number> NUMBER
%token <text> IDENT

%type <expr> expr
%type <stmt> stmt assign_stmt print_stmt if_stmt while_stmt func_stmt return_stmt guard_stmt
%type <stmts> stmt_list
%type <strs> param_list
%type <exprs> arg_list opt_arg_list

%left PIPE
%left EQ NEQ GT LT GTE LTE
%left PLUS MINUS
%left MUL DIV
%right UMINUS

%start program

%%

program
    : stmt_list { g_program = hs_make_program($1); }
    ;

stmt_list
    : /* empty */ { $$ = hs_empty_stmt_vec(); }
    | stmt_list stmt { hs_stmt_vec_push(&($1), $2); $$ = $1; }
    ;

stmt
    : assign_stmt { $$ = $1; }
    | print_stmt { $$ = $1; }
    | if_stmt { $$ = $1; }
    | guard_stmt { $$ = $1; }
    | while_stmt { $$ = $1; }
    | func_stmt { $$ = $1; }
    | return_stmt { $$ = $1; }
    ;

assign_stmt
    : RAKHO IDENT ME expr { $$ = hs_make_assign($2, $4, yylineno); }
    ;

print_stmt
    : LIKHO expr { $$ = hs_make_print($2, yylineno); }
    ;

if_stmt
    : AGAR expr TO SHURU stmt_list KHATAM {
        StmtVec empty_else = hs_empty_stmt_vec();
        $$ = hs_make_if($2, $5, empty_else, 0, yylineno);
      }
    | AGAR expr TO SHURU stmt_list KHATAM WARNA SHURU stmt_list KHATAM {
        $$ = hs_make_if($2, $5, $9, 1, yylineno);
      }
    ;

guard_stmt
    : JAB expr TO SHURU stmt_list KHATAM {
        StmtVec empty_else = hs_empty_stmt_vec();
        $$ = hs_make_if($2, $5, empty_else, 0, yylineno);
      }
    | JAB expr TO SHURU stmt_list KHATAM ANYA SHURU stmt_list KHATAM {
        $$ = hs_make_if($2, $5, $9, 1, yylineno);
      }
    | JAB expr TO SHURU stmt_list KHATAM ANYA guard_stmt {
        StmtVec else_wrap = hs_empty_stmt_vec();
        hs_stmt_vec_push(&else_wrap, $8);
        $$ = hs_make_if($2, $5, else_wrap, 1, yylineno);
      }
    ;

while_stmt
    : JABTAK expr SHURU stmt_list KHATAM { $$ = hs_make_while($2, $4, yylineno); }
    ;

func_stmt
    : BANAO IDENT param_list SHURU stmt_list KHATAM { $$ = hs_make_func($2, $3, $5, yylineno); }
    ;

param_list
    : /* empty */ { $$ = hs_empty_str_vec(); }
    | param_list IDENT { hs_str_vec_push(&($1), $2); $$ = $1; }
    ;

return_stmt
    : WAPAS expr { $$ = hs_make_return($2, yylineno); }
    ;

expr
    : NUMBER { $$ = hs_make_number($1, yylineno); }
    | IDENT { $$ = hs_make_ident($1, yylineno); }
    | IDENT LPAREN opt_arg_list RPAREN { $$ = hs_make_call($1, $3, yylineno); }
    | LPAREN expr RPAREN { $$ = $2; }
    | MINUS expr %prec UMINUS { $$ = hs_make_unary(OP_NEG, $2, yylineno); }
    | expr PLUS expr { $$ = hs_make_binary(OP_ADD, $1, $3, yylineno); }
    | expr MINUS expr { $$ = hs_make_binary(OP_SUB, $1, $3, yylineno); }
    | expr MUL expr { $$ = hs_make_binary(OP_MUL, $1, $3, yylineno); }
    | expr DIV expr { $$ = hs_make_binary(OP_DIV, $1, $3, yylineno); }
    | expr GT expr { $$ = hs_make_binary(OP_GT, $1, $3, yylineno); }
    | expr LT expr { $$ = hs_make_binary(OP_LT, $1, $3, yylineno); }
    | expr GTE expr { $$ = hs_make_binary(OP_GTE, $1, $3, yylineno); }
    | expr LTE expr { $$ = hs_make_binary(OP_LTE, $1, $3, yylineno); }
    | expr EQ expr { $$ = hs_make_binary(OP_EQ, $1, $3, yylineno); }
    | expr NEQ expr { $$ = hs_make_binary(OP_NEQ, $1, $3, yylineno); }
    | expr PIPE IDENT LPAREN opt_arg_list RPAREN {
        ExprVec v = hs_empty_expr_vec();
        int i;
        hs_expr_vec_push(&v, $1);
        for (i = 0; i < $5.len; i++) hs_expr_vec_push(&v, $5.items[i]);
        free($5.items);
        $$ = hs_make_call($3, v, yylineno);
      }
    ;

opt_arg_list
    : /* empty */ { $$ = hs_empty_expr_vec(); }
    | arg_list { $$ = $1; }
    ;

arg_list
    : expr {
        ExprVec v = hs_empty_expr_vec();
        hs_expr_vec_push(&v, $1);
        $$ = v;
      }
    | arg_list COMMA expr { hs_expr_vec_push(&($1), $3); $$ = $1; }
    ;

%%

Program* hs_parse_file(FILE* in) {
    g_program = NULL;
    g_parse_error = 0;
    yyrestart(in);
    if (yyparse() != 0) {
        g_parse_error = 1;
        if (g_program) {
            hs_free_program(g_program);
            g_program = NULL;
        }
    }
    if (g_parse_error) {
        if (g_program) {
            hs_free_program(g_program);
            g_program = NULL;
        }
        return NULL;
    }
    return g_program;
}

int hs_parse_had_error(void) {
    return g_parse_error;
}

