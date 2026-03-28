$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

$buildDir = 'build'
$bisonOut = 'build/parser.tab.c'
$flexOut = 'build/lexer.yy.c'
$exeOut = 'hindiscriptc.exe'

New-Item -ItemType Directory -Force $buildDir | Out-Null

Write-Host 'Generating parser...'
& bison -d -v -o $bisonOut src/compiler/parser.y
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
if (Test-Path parser.output) {
    Move-Item -Force parser.output build/parser.output
}

Write-Host 'Generating lexer...'
& flex src/compiler/lexer.l
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Move-Item -Force lex.yy.c $flexOut

Write-Host 'Compiling compiler...'
& gcc -std=c11 -O2 -Isrc/compiler -Ibuild -o $exeOut `
    src/compiler/main.c `
    src/compiler/ast.c `
    src/compiler/interpreter.c `
    src/compiler/codegen.c `
    $bisonOut `
    $flexOut `
    -lm
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "Built $exeOut"
Write-Host "Parser table report: build/parser.output"
