$ErrorActionPreference = 'Stop'

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1

Write-Host ''
Write-Host '--- Demo 1: Assign + Print (interpreter) ---'
& .\hindiscriptc.exe tests\ok\01_assign_print.hs --run-interpreter

Write-Host ''
Write-Host '--- Demo 2: Factorial (native codegen run) ---'
& .\hindiscriptc.exe tests\ok\09_factorial_demo.hs --run

Write-Host ''
Write-Host '--- Demo 3: Emit Generated C ---'
& .\hindiscriptc.exe tests\ok\02_expr_precedence.hs --emit-c

Write-Host ''
Write-Host '--- Error Demo 1: Missing me ---'
& .\hindiscriptc.exe tests\err\02_missing_me.hs --check
Write-Host "exit=$LASTEXITCODE"

Write-Host ''
Write-Host '--- Error Demo 2: Illegal character ---'
& .\hindiscriptc.exe tests\err\05_illegal_char.hs --check
Write-Host "exit=$LASTEXITCODE"
