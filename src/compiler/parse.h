#ifndef HS_PARSE_H
#define HS_PARSE_H

#include <stdio.h>
#include "ast.h"

Program* hs_parse_file(FILE* in);
int hs_parse_had_error(void);

#endif
