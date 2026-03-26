#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#endif

#include "ast.h"
#include "codegen.h"
#include "interpreter.h"
#include "lex.h"
#include "parse.h"

static int run_codegen_and_exec(const Program* program, const char* c_path, const char* exe_path) {
    FILE* c_out;
    char cmd[1024];
    int status;

    c_out = fopen(c_path, "wb");
    if (!c_out) {
        fprintf(stderr, "Error [codegen] line 0: cannot write '%s'\n", c_path);
        return 0;
    }
    if (!hs_emit_c_program(program, c_out)) {
        fclose(c_out);
        return 0;
    }
    fclose(c_out);

    snprintf(cmd, sizeof(cmd), "gcc -std=c11 -O2 -o \"%s\" \"%s\"", exe_path, c_path);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Error [codegen] line 0: gcc failed with status %d\n", status);
        return 0;
    }

    snprintf(cmd, sizeof(cmd), "\"%s\"", exe_path);
    status = system(cmd);
    if (status != 0) {
        fprintf(stderr, "Error [runtime] line 0: generated program failed with status %d\n", status);
        return 0;
    }

    remove(c_path);
    remove(exe_path);

    return 1;
}

static void usage(const char* prog) {
    fprintf(stderr, "Usage: %s <input.hs> [--check|--run|--run-interpreter|--emit-c|--emit-ast]\n", prog);
}

static int ensure_build_dir(void) {
#ifdef _WIN32
    if (_mkdir("build") == 0 || errno == EEXIST) return 1;
#else
    if (mkdir("build", 0777) == 0 || errno == EEXIST) return 1;
#endif
    return 0;
}

static int write_ast_file(const Program* program) {
    FILE* out;
    if (!ensure_build_dir()) {
        fprintf(stderr, "Error [io] line 0: cannot create 'build' directory\n");
        return 0;
    }

    out = fopen("build/ast.txt", "wb");
    if (!out) {
        fprintf(stderr, "Error [io] line 0: cannot write 'build/ast.txt'\n");
        return 0;
    }

    hs_print_ir(program, out);
    fclose(out);
    return 1;
}

int main(int argc, char** argv) {
    const char* input_path;
    const char* mode;
    FILE* in;
    Program* program;
    int ok = 1;

    if (argc < 2 || argc > 3) {
        usage(argv[0]);
        return 2;
    }

    input_path = argv[1];
    mode = (argc == 3) ? argv[2] : "--run";

    in = fopen(input_path, "rb");
    if (!in) {
        fprintf(stderr, "Error [io] line 0: cannot open '%s'\n", input_path);
        return 1;
    }

    hs_lex_reset_errors();
    program = hs_parse_file(in);
    fclose(in);

    if (!program || hs_lex_had_error() || hs_parse_had_error()) {
        if (program) hs_free_program(program);
        return 1;
    }

    if (strcmp(mode, "--check") == 0) {
        hs_free_program(program);
        return 0;
    }

    if (strcmp(mode, "--emit-ast") == 0) {
        ok = write_ast_file(program);
        hs_free_program(program);
        return ok ? 0 : 1;
    }

    if (!hs_semantic_check(program)) {
        hs_free_program(program);
        return 1;
    }

    if (strcmp(mode, "--run-interpreter") == 0) {
        ok = hs_interpret_program(program, stdout);
        hs_free_program(program);
        return ok ? 0 : 1;
    }

    if (strcmp(mode, "--emit-c") == 0) {
        ok = hs_emit_c_program(program, stdout);
        hs_free_program(program);
        return ok ? 0 : 1;
    }

    if (strcmp(mode, "--run") == 0) {
        ok = run_codegen_and_exec(program, ".hs_tmp_gen.c", ".hs_tmp_gen.exe");
        hs_free_program(program);
        return ok ? 0 : 1;
    }

    usage(argv[0]);
    hs_free_program(program);
    return 2;
}
