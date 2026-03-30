#ifndef HS_AST_H
#define HS_AST_H

#include <stdio.h>

typedef struct Expr Expr;
typedef struct Stmt Stmt;

typedef enum {
    EXPR_NUMBER,
    EXPR_IDENT,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL
} ExprKind;

typedef enum {
    STMT_ASSIGN,
    STMT_PRINT,
    STMT_PRINT_TEXT,
    STMT_IF,
    STMT_WHILE,
    STMT_FUNC,
    STMT_RETURN
} StmtKind;

typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_GT,
    OP_LT,
    OP_GTE,
    OP_LTE,
    OP_EQ,
    OP_NEQ,
    OP_NEG
} OpKind;

typedef struct {
    int len;
    int cap;
    Expr** items;
} ExprVec;

typedef struct {
    int len;
    int cap;
    char** items;
} StrVec;

typedef struct {
    int len;
    int cap;
    Stmt** items;
} StmtVec;

struct Expr {
    ExprKind kind;
    int line;
    union {
        double number;
        char* ident;
        struct {
            OpKind op;
            Expr* left;
            Expr* right;
        } binary;
        struct {
            OpKind op;
            Expr* operand;
        } unary;
        struct {
            char* callee;
            ExprVec args;
        } call;
    } as;
};

struct Stmt {
    StmtKind kind;
    int line;
    union {
        struct {
            char* name;
            Expr* value;
        } assign;
        struct {
            Expr* value;
        } print;
        struct {
            char* text;
        } print_text;
        struct {
            Expr* cond;
            StmtVec then_body;
            StmtVec else_body;
            int has_else;
        } if_stmt;
        struct {
            Expr* cond;
            StmtVec body;
        } while_stmt;
        struct {
            char* name;
            StrVec params;
            StmtVec body;
        } func;
        struct {
            Expr* value;
        } ret;
    } as;
};

typedef struct {
    StmtVec statements;
} Program;

char* hs_strdup(const char* s);

void hs_expr_vec_push(ExprVec* v, Expr* e);
void hs_str_vec_push(StrVec* v, char* s);
void hs_stmt_vec_push(StmtVec* v, Stmt* s);

Expr* hs_make_number(double value, int line);
Expr* hs_make_ident(char* name, int line);
Expr* hs_make_binary(OpKind op, Expr* left, Expr* right, int line);
Expr* hs_make_unary(OpKind op, Expr* operand, int line);
Expr* hs_make_call(char* callee, ExprVec args, int line);

Stmt* hs_make_assign(char* name, Expr* value, int line);
Stmt* hs_make_print(Expr* value, int line);
Stmt* hs_make_print_text(char* text, int line);
Stmt* hs_make_if(Expr* cond, StmtVec then_body, StmtVec else_body, int has_else, int line);
Stmt* hs_make_while(Expr* cond, StmtVec body, int line);
Stmt* hs_make_func(char* name, StrVec params, StmtVec body, int line);
Stmt* hs_make_return(Expr* value, int line);

Program* hs_make_program(StmtVec statements);

void hs_free_expr(Expr* e);
void hs_free_stmt(Stmt* s);
void hs_free_program(Program* p);

void hs_print_ir(const Program* p, FILE* out);

#endif
