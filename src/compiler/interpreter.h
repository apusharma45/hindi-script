#ifndef HS_INTERPRETER_H
#define HS_INTERPRETER_H

#include <stdio.h>
#include "ast.h"

int hs_semantic_check(const Program* p);
int hs_interpret_program(const Program* p, FILE* out);

#endif
