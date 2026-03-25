# Compiler Project Report (HindiScript MWL)

## Rubric Mapping

### 1. Lexical Analysis using Flex
- Implemented in `src/compiler/lexer.l`.
- Token coverage includes keywords, identifiers, numbers, operators, function-call punctuation.
- Invalid token handling:
  - Illegal characters: `Error [lexical] line <n>: illegal character ...`
  - Bad number format (`12.3.4`): explicit lexical error.

### 2. Syntax Analysis using Bison
- Implemented in `src/compiler/parser.y`.
- Grammar supports MWL core:
  - assignment: `rakho x me expr`
  - print: `likho expr`
  - if/else: `agar cond to shuru ... khatam [warna shuru ... khatam]`
  - while: `jabtak cond shuru ... khatam`
  - function definition: `banao name params... shuru ... khatam`
  - return: `wapas expr`
  - expressions with precedence + function calls
- Syntax errors include line number and phase tag.

### 3. Correctness of Execution Behavior
- AST representation in `src/compiler/ast.h/.c`.
- Semantic/runtime checks in `src/compiler/interpreter.c`:
  - undefined variable/function
  - duplicate function declaration
  - arity mismatch
  - return outside function
  - function missing return
- Correct execution path available in:
  - AST interpreter: `--run-interpreter`
  - Native C backend: `--run`

### 4. Unique / Advanced Feature
- Not included in the current build. Focus is on core lexer, parser, semantic/runtime correctness, and C backend execution.

## Test Evidence

Automated test script: `scripts/test.ps1`

It validates:
- OK suite (`tests/ok/*.hs`):
  - parse success
  - interpreter output matches expected outputs in `tests/expected/*.out`
  - native C backend output matches expected
  - interpreter/native parity
- ERR suite (`tests/err/*.hs`): expected failure (non-zero exit).
- semantic error suite (`tests/semerr/*.hs`): expected semantic/runtime failure (non-zero exit).
