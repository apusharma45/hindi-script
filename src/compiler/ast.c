#include "ast.h"

#include <stdlib.h>
#include <string.h>

static void* hs_malloc(size_t n) {
    void* p = malloc(n);
    if (!p) {
        fprintf(stderr, "Out of memory\n");
        exit(2);
    }
    return p;
}

char* hs_strdup(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)hs_malloc(n + 1);
    memcpy(out, s, n + 1);
    return out;
}

void hs_expr_vec_push(ExprVec* v, Expr* e) {
    if (v->len == v->cap) {
        v->cap = (v->cap == 0) ? 4 : v->cap * 2;
        v->items = (Expr**)realloc(v->items, sizeof(Expr*) * (size_t)v->cap);
    }
    v->items[v->len++] = e;
}

void hs_str_vec_push(StrVec* v, char* s) {
    if (v->len == v->cap) {
        v->cap = (v->cap == 0) ? 4 : v->cap * 2;
        v->items = (char**)realloc(v->items, sizeof(char*) * (size_t)v->cap);
    }
    v->items[v->len++] = s;
}

void hs_stmt_vec_push(StmtVec* v, Stmt* s) {
    if (v->len == v->cap) {
        v->cap = (v->cap == 0) ? 8 : v->cap * 2;
        v->items = (Stmt**)realloc(v->items, sizeof(Stmt*) * (size_t)v->cap);
    }
    v->items[v->len++] = s;
}

Expr* hs_make_number(double value, int line) {
    Expr* e = (Expr*)hs_malloc(sizeof(Expr));
    e->kind = EXPR_NUMBER;
    e->line = line;
    e->as.number = value;
    return e;
}

Expr* hs_make_ident(char* name, int line) {
    Expr* e = (Expr*)hs_malloc(sizeof(Expr));
    e->kind = EXPR_IDENT;
    e->line = line;
    e->as.ident = name;
    return e;
}

Expr* hs_make_binary(OpKind op, Expr* left, Expr* right, int line) {
    Expr* e = (Expr*)hs_malloc(sizeof(Expr));
    e->kind = EXPR_BINARY;
    e->line = line;
    e->as.binary.op = op;
    e->as.binary.left = left;
    e->as.binary.right = right;
    return e;
}

Expr* hs_make_unary(OpKind op, Expr* operand, int line) {
    Expr* e = (Expr*)hs_malloc(sizeof(Expr));
    e->kind = EXPR_UNARY;
    e->line = line;
    e->as.unary.op = op;
    e->as.unary.operand = operand;
    return e;
}

Expr* hs_make_call(char* callee, ExprVec args, int line) {
    Expr* e = (Expr*)hs_malloc(sizeof(Expr));
    e->kind = EXPR_CALL;
    e->line = line;
    e->as.call.callee = callee;
    e->as.call.args = args;
    return e;
}

Stmt* hs_make_assign(char* name, Expr* value, int line) {
    Stmt* s = (Stmt*)hs_malloc(sizeof(Stmt));
    s->kind = STMT_ASSIGN;
    s->line = line;
    s->as.assign.name = name;
    s->as.assign.value = value;
    return s;
}

Stmt* hs_make_print(Expr* value, int line) {
    Stmt* s = (Stmt*)hs_malloc(sizeof(Stmt));
    s->kind = STMT_PRINT;
    s->line = line;
    s->as.print.value = value;
    return s;
}

Stmt* hs_make_print_text(char* text, int line) {
    Stmt* s = (Stmt*)hs_malloc(sizeof(Stmt));
    s->kind = STMT_PRINT_TEXT;
    s->line = line;
    s->as.print_text.text = text;
    return s;
}

Stmt* hs_make_if(Expr* cond, StmtVec then_body, StmtVec else_body, int has_else, int line) {
    Stmt* s = (Stmt*)hs_malloc(sizeof(Stmt));
    s->kind = STMT_IF;
    s->line = line;
    s->as.if_stmt.cond = cond;
    s->as.if_stmt.then_body = then_body;
    s->as.if_stmt.else_body = else_body;
    s->as.if_stmt.has_else = has_else;
    return s;
}

Stmt* hs_make_while(Expr* cond, StmtVec body, int line) {
    Stmt* s = (Stmt*)hs_malloc(sizeof(Stmt));
    s->kind = STMT_WHILE;
    s->line = line;
    s->as.while_stmt.cond = cond;
    s->as.while_stmt.body = body;
    return s;
}

Stmt* hs_make_func(char* name, StrVec params, StmtVec body, int line) {
    Stmt* s = (Stmt*)hs_malloc(sizeof(Stmt));
    s->kind = STMT_FUNC;
    s->line = line;
    s->as.func.name = name;
    s->as.func.params = params;
    s->as.func.body = body;
    return s;
}

Stmt* hs_make_return(Expr* value, int line) {
    Stmt* s = (Stmt*)hs_malloc(sizeof(Stmt));
    s->kind = STMT_RETURN;
    s->line = line;
    s->as.ret.value = value;
    return s;
}

Program* hs_make_program(StmtVec statements) {
    Program* p = (Program*)hs_malloc(sizeof(Program));
    p->statements = statements;
    return p;
}

void hs_free_expr(Expr* e) {
    int i;
    if (!e) return;
    switch (e->kind) {
        case EXPR_IDENT:
            free(e->as.ident);
            break;
        case EXPR_BINARY:
            hs_free_expr(e->as.binary.left);
            hs_free_expr(e->as.binary.right);
            break;
        case EXPR_UNARY:
            hs_free_expr(e->as.unary.operand);
            break;
        case EXPR_CALL:
            free(e->as.call.callee);
            for (i = 0; i < e->as.call.args.len; i++) hs_free_expr(e->as.call.args.items[i]);
            free(e->as.call.args.items);
            break;
        default:
            break;
    }
    free(e);
}

void hs_free_stmt(Stmt* s) {
    int i;
    if (!s) return;
    switch (s->kind) {
        case STMT_ASSIGN:
            free(s->as.assign.name);
            hs_free_expr(s->as.assign.value);
            break;
        case STMT_PRINT:
            hs_free_expr(s->as.print.value);
            break;
        case STMT_PRINT_TEXT:
            free(s->as.print_text.text);
            break;
        case STMT_IF:
            hs_free_expr(s->as.if_stmt.cond);
            for (i = 0; i < s->as.if_stmt.then_body.len; i++) hs_free_stmt(s->as.if_stmt.then_body.items[i]);
            for (i = 0; i < s->as.if_stmt.else_body.len; i++) hs_free_stmt(s->as.if_stmt.else_body.items[i]);
            free(s->as.if_stmt.then_body.items);
            free(s->as.if_stmt.else_body.items);
            break;
        case STMT_WHILE:
            hs_free_expr(s->as.while_stmt.cond);
            for (i = 0; i < s->as.while_stmt.body.len; i++) hs_free_stmt(s->as.while_stmt.body.items[i]);
            free(s->as.while_stmt.body.items);
            break;
        case STMT_FUNC:
            free(s->as.func.name);
            for (i = 0; i < s->as.func.params.len; i++) free(s->as.func.params.items[i]);
            free(s->as.func.params.items);
            for (i = 0; i < s->as.func.body.len; i++) hs_free_stmt(s->as.func.body.items[i]);
            free(s->as.func.body.items);
            break;
        case STMT_RETURN:
            hs_free_expr(s->as.ret.value);
            break;
    }
    free(s);
}

void hs_free_program(Program* p) {
    int i;
    if (!p) return;
    for (i = 0; i < p->statements.len; i++) hs_free_stmt(p->statements.items[i]);
    free(p->statements.items);
    free(p);
}

static const char* hs_op_name(OpKind op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_GT: return ">";
        case OP_LT: return "<";
        case OP_GTE: return ">=";
        case OP_LTE: return "<=";
        case OP_EQ: return "==";
        case OP_NEQ: return "!=";
        case OP_NEG: return "neg";
        default: return "?";
    }
}

static void hs_indent(FILE* out, int n) {
    int i;
    for (i = 0; i < n; i++) fputs("  ", out);
}

static void hs_ir_expr(const Expr* e, FILE* out, int depth) {
    int i;
    hs_indent(out, depth);
    switch (e->kind) {
        case EXPR_NUMBER:
            fprintf(out, "num %g\n", e->as.number);
            break;
        case EXPR_IDENT:
            fprintf(out, "id %s\n", e->as.ident);
            break;
        case EXPR_UNARY:
            fprintf(out, "unary %s\n", hs_op_name(e->as.unary.op));
            hs_ir_expr(e->as.unary.operand, out, depth + 1);
            break;
        case EXPR_BINARY:
            fprintf(out, "binary %s\n", hs_op_name(e->as.binary.op));
            hs_ir_expr(e->as.binary.left, out, depth + 1);
            hs_ir_expr(e->as.binary.right, out, depth + 1);
            break;
        case EXPR_CALL:
            fprintf(out, "call %s\n", e->as.call.callee);
            for (i = 0; i < e->as.call.args.len; i++) hs_ir_expr(e->as.call.args.items[i], out, depth + 1);
            break;
    }
}

static void hs_ir_stmts(const StmtVec* v, FILE* out, int depth) {
    int i;
    for (i = 0; i < v->len; i++) {
        const Stmt* s = v->items[i];
        hs_indent(out, depth);
        switch (s->kind) {
            case STMT_ASSIGN:
                fprintf(out, "assign %s\n", s->as.assign.name);
                hs_ir_expr(s->as.assign.value, out, depth + 1);
                break;
            case STMT_PRINT:
                fputs("print\n", out);
                hs_ir_expr(s->as.print.value, out, depth + 1);
                break;
            case STMT_PRINT_TEXT:
                fprintf(out, "print_text \"%s\"\n", s->as.print_text.text);
                break;
            case STMT_RETURN:
                fputs("return\n", out);
                hs_ir_expr(s->as.ret.value, out, depth + 1);
                break;
            case STMT_WHILE:
                fputs("while\n", out);
                hs_ir_expr(s->as.while_stmt.cond, out, depth + 1);
                hs_ir_stmts(&s->as.while_stmt.body, out, depth + 1);
                break;
            case STMT_IF:
                fputs("if\n", out);
                hs_ir_expr(s->as.if_stmt.cond, out, depth + 1);
                hs_indent(out, depth + 1);
                fputs("then\n", out);
                hs_ir_stmts(&s->as.if_stmt.then_body, out, depth + 2);
                if (s->as.if_stmt.has_else) {
                    hs_indent(out, depth + 1);
                    fputs("else\n", out);
                    hs_ir_stmts(&s->as.if_stmt.else_body, out, depth + 2);
                }
                break;
            case STMT_FUNC:
                fprintf(out, "func %s\n", s->as.func.name);
                hs_ir_stmts(&s->as.func.body, out, depth + 1);
                break;
        }
    }
}

void hs_print_ir(const Program* p, FILE* out) {
    hs_ir_stmts(&p->statements, out, 0);
}
