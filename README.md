# HindiScript Compiler (MWL Core)

## Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
```

Build artifacts:
- `build/parser.tab.c`
- `build/parser.tab.h`
- `build/lexer.yy.c`
- `build/parser.output` (Bison state/parse table report)

## Test

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test.ps1
```

## CLI

```text
hindiscriptc <input.hs> [--check|--run|--run-interpreter|--emit-c|--emit-ast]
```

- `--check`: lexical + syntax validation only.
- `--run`: generate C with `gcc`, execute native binary.
- `--run-interpreter`: execute AST interpreter.
- `--emit-c`: print generated C to stdout.
- `--emit-ast`: write AST dump to `build/ast.txt`.

## Language Notes

Implemented core statements and expressions, plus:
- Pipeline operator: `expr |> func(...)`
- Guard rules: `jab ...`, `anya jab ...`, `anya ...`

Test suites:
- `tests/ok`: valid programs and expected outputs
- `tests/err`: lexical/syntax failure cases
- `tests/semerr`: semantic/runtime validation failures

## Quick Demo

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/demo.ps1
```
