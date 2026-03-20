#ifndef HS_CODEGEN_H
#define HS_CODEGEN_H

#include <stdio.h>
#include "ast.h"

int hs_emit_c_program(const Program* p, FILE* out);

#endif
