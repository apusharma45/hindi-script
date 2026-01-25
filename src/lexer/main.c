#include <stdio.h>

int yylex(void);

int main(void) {
    int r = yylex();
    return (r == 0) ? 0 : 1;
}
