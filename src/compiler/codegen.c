#include "codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int len;
    int cap;
    char** items;
} NameVec;

static void names_push_unique(NameVec* v, const char* s) {
    int i;
    for (i = 0; i < v->len; i++) if (strcmp(v->items[i], s) == 0) return;
    if (v->len == v->cap) {
        v->cap = (v->cap == 0) ? 8 : v->cap * 2;
        v->items = (char**)realloc(v->items, sizeof(char*) * (size_t)v->cap);
    }
    v->items[v->len++] = hs_strdup(s);
}

static void names_free(NameVec* v) {
    int i;
    for (i = 0; i < v->len; i++) free(v->items[i]);
    free(v->items);
}

static void emit_expr(const Expr* e, FILE* out) {
    int i;
    switch (e->kind) {
        case EXPR_NUMBER:
            fprintf(out, "%g", e->as.number);
            break;
        case EXPR_IDENT:
            fprintf(out, "%s", e->as.ident);
            break;
        case EXPR_UNARY:
            if (e->as.unary.op == OP_NEG) {
                fputs("(-", out);
                emit_expr(e->as.unary.operand, out);
                fputc(')', out);
            }
            break;
        case EXPR_BINARY:
            fputc('(', out);
            emit_expr(e->as.binary.left, out);
            switch (e->as.binary.op) {
                case OP_ADD: fputs(" + ", out); break;
                case OP_SUB: fputs(" - ", out); break;
                case OP_MUL: fputs(" * ", out); break;
                case OP_DIV: fputs(" / ", out); break;
                case OP_GT: fputs(" > ", out); break;
                case OP_LT: fputs(" < ", out); break;
                case OP_GTE: fputs(" >= ", out); break;
                case OP_LTE: fputs(" <= ", out); break;
                case OP_EQ: fputs(" == ", out); break;
                case OP_NEQ: fputs(" != ", out); break;
                default: break;
            }
            emit_expr(e->as.binary.right, out);
            fputc(')', out);
            break;
        case EXPR_CALL:
            fprintf(out, "%s(", e->as.call.callee);
            for (i = 0; i < e->as.call.args.len; i++) {
                if (i) fputs(", ", out);
                emit_expr(e->as.call.args.items[i], out);
            }
            fputc(')', out);
            break;
    }
}

static void collect_stmt_vars(const Stmt* s, NameVec* vars) {
    int i;
    switch (s->kind) {
        case STMT_ASSIGN:
            names_push_unique(vars, s->as.assign.name);
            break;
        case STMT_IF:
            for (i = 0; i < s->as.if_stmt.then_body.len; i++) collect_stmt_vars(s->as.if_stmt.then_body.items[i], vars);
            for (i = 0; i < s->as.if_stmt.else_body.len; i++) collect_stmt_vars(s->as.if_stmt.else_body.items[i], vars);
            break;
        case STMT_WHILE:
            for (i = 0; i < s->as.while_stmt.body.len; i++) collect_stmt_vars(s->as.while_stmt.body.items[i], vars);
            break;
        default:
            break;
    }
}

static void emit_indent(FILE* out, int depth) {
    int i;
    for (i = 0; i < depth; i++) fputs("    ", out);
}

static void emit_stmt_list(const StmtVec* list, FILE* out, int depth) {
    int i;
    for (i = 0; i < list->len; i++) {
        const Stmt* s = list->items[i];
        if (s->kind == STMT_FUNC) continue;
        emit_indent(out, depth);
        switch (s->kind) {
            case STMT_ASSIGN:
                fprintf(out, "%s = ", s->as.assign.name);
                emit_expr(s->as.assign.value, out);
                fputs(";\n", out);
                break;
            case STMT_PRINT:
                fputs("printf(\"%g\\n\", (double)(", out);
                emit_expr(s->as.print.value, out);
                fputs("));\n", out);
                break;
            case STMT_RETURN:
                fputs("return ", out);
                emit_expr(s->as.ret.value, out);
                fputs(";\n", out);
                break;
            case STMT_IF:
                fputs("if (", out);
                emit_expr(s->as.if_stmt.cond, out);
                fputs(") {\n", out);
                emit_stmt_list(&s->as.if_stmt.then_body, out, depth + 1);
                emit_indent(out, depth);
                if (s->as.if_stmt.has_else) {
                    fputs("} else {\n", out);
                    emit_stmt_list(&s->as.if_stmt.else_body, out, depth + 1);
                    emit_indent(out, depth);
                }
                fputs("}\n", out);
                break;
            case STMT_WHILE:
                fputs("while (", out);
                emit_expr(s->as.while_stmt.cond, out);
                fputs(") {\n", out);
                emit_stmt_list(&s->as.while_stmt.body, out, depth + 1);
                emit_indent(out, depth);
                fputs("}\n", out);
                break;
            default:
                break;
        }
    }
}

static void emit_var_decls(const StmtVec* list, FILE* out, int depth, const StrVec* params) {
    NameVec vars;
    int i;
    vars.len = 0;
    vars.cap = 0;
    vars.items = NULL;

    for (i = 0; i < list->len; i++) collect_stmt_vars(list->items[i], &vars);

    for (i = 0; i < vars.len; i++) {
        int is_param = 0;
        int p;
        if (params) {
            for (p = 0; p < params->len; p++) {
                if (strcmp(params->items[p], vars.items[i]) == 0) {
                    is_param = 1;
                    break;
                }
            }
        }
        if (!is_param) {
            emit_indent(out, depth);
            fprintf(out, "double %s = 0;\n", vars.items[i]);
        }
    }

    names_free(&vars);
}

int hs_emit_c_program(const Program* p, FILE* out) {
    int i, j;
    fputs("#include <stdio.h>\n\n", out);

    for (i = 0; i < p->statements.len; i++) {
        const Stmt* s = p->statements.items[i];
        if (s->kind != STMT_FUNC) continue;
        fprintf(out, "double %s(", s->as.func.name);
        for (j = 0; j < s->as.func.params.len; j++) {
            if (j) fputs(", ", out);
            fprintf(out, "double %s", s->as.func.params.items[j]);
        }
        fputs(") {\n", out);
        emit_var_decls(&s->as.func.body, out, 1, &s->as.func.params);
        emit_stmt_list(&s->as.func.body, out, 1);
        fputs("    return 0;\n", out);
        fputs("}\n\n", out);
    }

    fputs("int main(void) {\n", out);
    emit_var_decls(&p->statements, out, 1, NULL);
    emit_stmt_list(&p->statements, out, 1);
    fputs("    return 0;\n", out);
    fputs("}\n", out);
    return 1;
}

