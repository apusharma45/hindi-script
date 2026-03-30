#include "interpreter.h"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Scope Scope;
typedef struct FunctionDef FunctionDef;

typedef struct {
    char* name;
    double value;
} Var;

struct Scope {
    Scope* parent;
    int len;
    int cap;
    Var* vars;
};

struct FunctionDef {
    const Stmt* stmt;
};

typedef struct {
    const Program* program;
    int has_error;
    int in_function;
    FILE* out;
    int func_len;
    int func_cap;
    char** func_names;
    FunctionDef* funcs;
} Context;

typedef struct {
    int has_return;
    double value;
} ExecSignal;

static void hs_error(Context* ctx, int line, const char* fmt, ...) {
    va_list ap;
    fprintf(stderr, "Error [semantic] line %d: ", line);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    ctx->has_error = 1;
}

static void scope_init(Scope* s, Scope* parent) {
    s->parent = parent;
    s->len = 0;
    s->cap = 0;
    s->vars = NULL;
}

static void scope_free(Scope* s) {
    int i;
    for (i = 0; i < s->len; i++) free(s->vars[i].name);
    free(s->vars);
}

static int scope_find_local(const Scope* s, const char* name) {
    int i;
    for (i = 0; i < s->len; i++) if (strcmp(s->vars[i].name, name) == 0) return i;
    return -1;
}

static Scope* scope_find_any(Scope* s, const char* name, int* idx) {
    Scope* cur = s;
    while (cur) {
        int i = scope_find_local(cur, name);
        if (i >= 0) {
            *idx = i;
            return cur;
        }
        cur = cur->parent;
    }
    return NULL;
}

static void scope_set(Scope* s, const char* name, double value) {
    int idx = -1;
    Scope* owner = scope_find_any(s, name, &idx);
    if (owner) {
        owner->vars[idx].value = value;
        return;
    }
    if (s->len == s->cap) {
        s->cap = (s->cap == 0) ? 8 : s->cap * 2;
        s->vars = (Var*)realloc(s->vars, sizeof(Var) * (size_t)s->cap);
    }
    s->vars[s->len].name = hs_strdup(name);
    s->vars[s->len].value = value;
    s->len++;
}

static int scope_get(Scope* s, const char* name, double* out) {
    int idx = -1;
    Scope* owner = scope_find_any(s, name, &idx);
    if (!owner) return 0;
    *out = owner->vars[idx].value;
    return 1;
}

static int func_find(Context* ctx, const char* name) {
    int i;
    for (i = 0; i < ctx->func_len; i++) if (strcmp(ctx->func_names[i], name) == 0) return i;
    return -1;
}

static void func_add(Context* ctx, const char* name, const Stmt* fn_stmt) {
    if (ctx->func_len == ctx->func_cap) {
        ctx->func_cap = (ctx->func_cap == 0) ? 8 : ctx->func_cap * 2;
        ctx->func_names = (char**)realloc(ctx->func_names, sizeof(char*) * (size_t)ctx->func_cap);
        ctx->funcs = (FunctionDef*)realloc(ctx->funcs, sizeof(FunctionDef) * (size_t)ctx->func_cap);
    }
    ctx->func_names[ctx->func_len] = hs_strdup(name);
    ctx->funcs[ctx->func_len].stmt = fn_stmt;
    ctx->func_len++;
}

static int collect_functions(Context* ctx) {
    int i;
    for (i = 0; i < ctx->program->statements.len; i++) {
        const Stmt* s = ctx->program->statements.items[i];
        if (s->kind != STMT_FUNC) continue;
        if (strcmp(s->as.func.name, "padho") == 0) {
            hs_error(ctx, s->line, "function name 'padho' is reserved for builtin input");
            return 0;
        }
        if (func_find(ctx, s->as.func.name) >= 0) {
            hs_error(ctx, s->line, "duplicate function '%s'", s->as.func.name);
            return 0;
        }
        func_add(ctx, s->as.func.name, s);
    }
    return !ctx->has_error;
}

static int eval_expr(Context* ctx, Scope* scope, const Expr* e, double* out);

static int eval_call(Context* ctx, Scope* caller_scope, const Expr* e, double* out) {
    int i, idx;
    Scope fn_scope;
    ExecSignal sig;
    const Stmt* fn_stmt;

    if (strcmp(e->as.call.callee, "padho") == 0) {
        if (e->as.call.args.len != 0) {
            hs_error(ctx, e->line, "arity mismatch for 'padho': expected 0, got %d", e->as.call.args.len);
            return 0;
        }
        if (!ctx->out) {
            *out = 0.0;
            return 1;
        }
        if (scanf("%lf", out) != 1) {
            *out = 0.0;
        }
        return 1;
    }

    idx = func_find(ctx, e->as.call.callee);
    if (idx < 0) {
        hs_error(ctx, e->line, "undefined function '%s'", e->as.call.callee);
        return 0;
    }

    fn_stmt = ctx->funcs[idx].stmt;
    if (fn_stmt->as.func.params.len != e->as.call.args.len) {
        hs_error(ctx, e->line, "arity mismatch for '%s': expected %d, got %d", e->as.call.callee,
                 fn_stmt->as.func.params.len, e->as.call.args.len);
        return 0;
    }

    scope_init(&fn_scope, caller_scope);
    for (i = 0; i < fn_stmt->as.func.params.len; i++) {
        double v = 0.0;
        if (!eval_expr(ctx, caller_scope, e->as.call.args.items[i], &v)) {
            scope_free(&fn_scope);
            return 0;
        }
        scope_set(&fn_scope, fn_stmt->as.func.params.items[i], v);
    }

    sig.has_return = 0;
    sig.value = 0.0;
    {
        int old_in_fn = ctx->in_function;
        int j;
        ctx->in_function = 1;
        for (j = 0; j < fn_stmt->as.func.body.len; j++) {
            const Stmt* s = fn_stmt->as.func.body.items[j];
            if (s->kind == STMT_FUNC) {
                hs_error(ctx, s->line, "nested function definitions are not allowed");
                break;
            }
            if (!ctx->has_error) {
                ExecSignal inner;
                inner.has_return = 0;
                inner.value = 0.0;
                if (s->kind == STMT_RETURN) {
                    if (!eval_expr(ctx, &fn_scope, s->as.ret.value, &inner.value)) break;
                    inner.has_return = 1;
                } else if (s->kind == STMT_ASSIGN) {
                    double v = 0.0;
                    if (!eval_expr(ctx, &fn_scope, s->as.assign.value, &v)) break;
                    scope_set(&fn_scope, s->as.assign.name, v);
                } else if (s->kind == STMT_PRINT) {
                    double v = 0.0;
                    if (!eval_expr(ctx, &fn_scope, s->as.print.value, &v)) break;
                    if (ctx->out) fprintf(ctx->out, "%g\n", v);
                } else if (s->kind == STMT_PRINT_TEXT) {
                    if (ctx->out) fprintf(ctx->out, "%s\n", s->as.print_text.text);
                } else if (s->kind == STMT_IF) {
                    double cond = 0.0;
                    int k;
                    if (!eval_expr(ctx, &fn_scope, s->as.if_stmt.cond, &cond)) break;
                    {
                        const StmtVec* body = (cond != 0.0) ? &s->as.if_stmt.then_body : &s->as.if_stmt.else_body;
                        int run_body = (cond != 0.0) || s->as.if_stmt.has_else;
                        if (run_body) {
                            for (k = 0; k < body->len; k++) {
                                const Stmt* bs = body->items[k];
                                if (bs->kind == STMT_RETURN) {
                                    if (!eval_expr(ctx, &fn_scope, bs->as.ret.value, &inner.value)) break;
                                    inner.has_return = 1;
                                    break;
                                }
                                if (bs->kind == STMT_ASSIGN) {
                                    double bv;
                                    if (!eval_expr(ctx, &fn_scope, bs->as.assign.value, &bv)) break;
                                    scope_set(&fn_scope, bs->as.assign.name, bv);
                                } else if (bs->kind == STMT_PRINT) {
                                    double bv;
                                    if (!eval_expr(ctx, &fn_scope, bs->as.print.value, &bv)) break;
                                    if (ctx->out) fprintf(ctx->out, "%g\n", bv);
                                } else if (bs->kind == STMT_PRINT_TEXT) {
                                    if (ctx->out) fprintf(ctx->out, "%s\n", bs->as.print_text.text);
                                }
                            }
                        }
                    }
                } else if (s->kind == STMT_WHILE) {
                    int guard = 0;
                    while (!ctx->has_error) {
                        double cond = 0.0;
                        int k;
                        if (++guard > 1000000) {
                            hs_error(ctx, s->line, "loop guard triggered");
                            break;
                        }
                        if (!eval_expr(ctx, &fn_scope, s->as.while_stmt.cond, &cond)) break;
                        if (cond == 0.0) break;
                        for (k = 0; k < s->as.while_stmt.body.len; k++) {
                            const Stmt* bs = s->as.while_stmt.body.items[k];
                            if (bs->kind == STMT_RETURN) {
                                if (!eval_expr(ctx, &fn_scope, bs->as.ret.value, &inner.value)) break;
                                inner.has_return = 1;
                                break;
                            }
                            if (bs->kind == STMT_ASSIGN) {
                                double bv;
                                if (!eval_expr(ctx, &fn_scope, bs->as.assign.value, &bv)) break;
                                scope_set(&fn_scope, bs->as.assign.name, bv);
                            } else if (bs->kind == STMT_PRINT) {
                                double bv;
                                if (!eval_expr(ctx, &fn_scope, bs->as.print.value, &bv)) break;
                                if (ctx->out) fprintf(ctx->out, "%g\n", bv);
                            } else if (bs->kind == STMT_PRINT_TEXT) {
                                if (ctx->out) fprintf(ctx->out, "%s\n", bs->as.print_text.text);
                            }
                        }
                        if (inner.has_return || ctx->has_error) break;
                    }
                }
                if (inner.has_return) {
                    sig = inner;
                    break;
                }
            }
            if (ctx->has_error) break;
        }
        ctx->in_function = old_in_fn;
    }

    if (!ctx->has_error && !sig.has_return) {
        hs_error(ctx, fn_stmt->line, "function '%s' did not return a value", fn_stmt->as.func.name);
    }

    scope_free(&fn_scope);
    if (ctx->has_error) return 0;
    *out = sig.value;
    return 1;
}

static int eval_expr(Context* ctx, Scope* scope, const Expr* e, double* out) {
    double l = 0.0;
    double r = 0.0;
    switch (e->kind) {
        case EXPR_NUMBER:
            *out = e->as.number;
            return 1;
        case EXPR_IDENT:
            if (!scope_get(scope, e->as.ident, out)) {
                hs_error(ctx, e->line, "undefined variable '%s'", e->as.ident);
                return 0;
            }
            return 1;
        case EXPR_UNARY:
            if (!eval_expr(ctx, scope, e->as.unary.operand, out)) return 0;
            if (e->as.unary.op == OP_NEG) *out = -(*out);
            return 1;
        case EXPR_BINARY:
            if (!eval_expr(ctx, scope, e->as.binary.left, &l)) return 0;
            if (!eval_expr(ctx, scope, e->as.binary.right, &r)) return 0;
            switch (e->as.binary.op) {
                case OP_ADD: *out = l + r; return 1;
                case OP_SUB: *out = l - r; return 1;
                case OP_MUL: *out = l * r; return 1;
                case OP_DIV:
                    if (fabs(r) < 1e-12) {
                        hs_error(ctx, e->line, "division by zero");
                        return 0;
                    }
                    *out = l / r;
                    return 1;
                case OP_GT: *out = (l > r) ? 1.0 : 0.0; return 1;
                case OP_LT: *out = (l < r) ? 1.0 : 0.0; return 1;
                case OP_GTE: *out = (l >= r) ? 1.0 : 0.0; return 1;
                case OP_LTE: *out = (l <= r) ? 1.0 : 0.0; return 1;
                case OP_EQ: *out = (fabs(l - r) < 1e-12) ? 1.0 : 0.0; return 1;
                case OP_NEQ: *out = (fabs(l - r) >= 1e-12) ? 1.0 : 0.0; return 1;
                default: return 0;
            }
        case EXPR_CALL:
            return eval_call(ctx, scope, e, out);
    }
    return 0;
}

static int exec_block(Context* ctx, Scope* scope, const StmtVec* body, ExecSignal* sig) {
    int i;
    sig->has_return = 0;
    sig->value = 0.0;

    for (i = 0; i < body->len; i++) {
        const Stmt* s = body->items[i];
        if (s->kind == STMT_FUNC) {
            hs_error(ctx, s->line, "function definitions are only allowed at top level");
            return 0;
        }
        if (s->kind == STMT_RETURN) {
            if (!ctx->in_function) {
                hs_error(ctx, s->line, "return used outside function");
                return 0;
            }
            if (!eval_expr(ctx, scope, s->as.ret.value, &sig->value)) return 0;
            sig->has_return = 1;
            return 1;
        }
        if (s->kind == STMT_ASSIGN) {
            double v;
            if (!eval_expr(ctx, scope, s->as.assign.value, &v)) return 0;
            scope_set(scope, s->as.assign.name, v);
            continue;
        }
        if (s->kind == STMT_PRINT) {
            double v;
            if (!eval_expr(ctx, scope, s->as.print.value, &v)) return 0;
            if (ctx->out) fprintf(ctx->out, "%g\n", v);
            continue;
        }
        if (s->kind == STMT_PRINT_TEXT) {
            if (ctx->out) fprintf(ctx->out, "%s\n", s->as.print_text.text);
            continue;
        }
        if (s->kind == STMT_IF) {
            double cond;
            const StmtVec* next_body;
            if (!eval_expr(ctx, scope, s->as.if_stmt.cond, &cond)) return 0;
            if (cond != 0.0) {
                next_body = &s->as.if_stmt.then_body;
            } else if (s->as.if_stmt.has_else) {
                next_body = &s->as.if_stmt.else_body;
            } else {
                continue;
            }
            if (!exec_block(ctx, scope, next_body, sig)) return 0;
            if (sig->has_return) return 1;
            continue;
        }
        if (s->kind == STMT_WHILE) {
            int guard = 0;
            while (1) {
                double cond;
                if (++guard > 1000000) {
                    hs_error(ctx, s->line, "loop guard triggered");
                    return 0;
                }
                if (!eval_expr(ctx, scope, s->as.while_stmt.cond, &cond)) return 0;
                if (cond == 0.0) break;
                if (!exec_block(ctx, scope, &s->as.while_stmt.body, sig)) return 0;
                if (sig->has_return) return 1;
            }
            continue;
        }
    }
    return 1;
}

static int run(Context* ctx, FILE* out) {
    Scope global_scope;
    ExecSignal sig;

    scope_init(&global_scope, NULL);
    ctx->out = out;
    ctx->in_function = 0;

    if (!collect_functions(ctx)) {
        scope_free(&global_scope);
        return 0;
    }

    {
        int i;
        for (i = 0; i < ctx->program->statements.len; i++) {
            const Stmt* s = ctx->program->statements.items[i];
            if (s->kind == STMT_FUNC) continue;
            {
                StmtVec one;
                Stmt* temp = (Stmt*)s;
                one.len = 1;
                one.cap = 1;
                one.items = &temp;
                if (!exec_block(ctx, &global_scope, &one, &sig)) {
                    scope_free(&global_scope);
                    return 0;
                }
                if (sig.has_return) {
                    hs_error(ctx, s->line, "return used outside function");
                    scope_free(&global_scope);
                    return 0;
                }
            }
        }
    }

    scope_free(&global_scope);
    return !ctx->has_error;
}

static int run_with_output(const Program* p, FILE* out) {
    Context ctx;
    int ok;
    int i;
    ctx.program = p;
    ctx.has_error = 0;
    ctx.in_function = 0;
    ctx.out = out;
    ctx.func_len = 0;
    ctx.func_cap = 0;
    ctx.func_names = NULL;
    ctx.funcs = NULL;

    ok = run(&ctx, out);

    for (i = 0; i < ctx.func_len; i++) free(ctx.func_names[i]);
    free(ctx.func_names);
    free(ctx.funcs);
    return ok;
}

int hs_semantic_check(const Program* p) {
    return run_with_output(p, NULL);
}

int hs_interpret_program(const Program* p, FILE* out) {
    return run_with_output(p, out);
}
