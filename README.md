# HindiScript Compiler (MWL Core)

## Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
```

## Test

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test.ps1
```

## CLI

```text
hindiscriptc <input.hs> [--check|--run|--run-interpreter|--emit-c]
```

- `--check`: lexical + syntax validation only.
- `--run`: generate C with `gcc`, execute native binary.
- `--run-interpreter`: execute AST interpreter.
- `--emit-c`: print generated C to stdout.

Test suites:
- `tests/ok`: valid programs and expected outputs
- `tests/err`: lexical/syntax failure cases
- `tests/semerr`: semantic/runtime validation failures

## Quick Demo

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/demo.ps1
```
