$ErrorActionPreference = 'Stop'
if (Get-Variable -Name PSNativeCommandUseErrorActionPreference -ErrorAction SilentlyContinue) {
    $PSNativeCommandUseErrorActionPreference = $false
}

$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

& powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$exe = Join-Path $root 'hindiscriptc.exe'
$failed = $false

function Run-And-Capture([string]$file, [string]$mode) {
    $output = & $exe $file $mode 2>$null
    $code = $LASTEXITCODE
    return @{ output = ($output -join "`n"); code = $code }
}

function Normalize-Text([string]$s) {
    if ($null -eq $s) { return '' }
    return (($s -replace "`r`n", "`n" -replace "`r", "`n").TrimEnd("`n"))
}

Write-Host 'Running OK tests...'
Get-ChildItem tests/ok/*.hs | Sort-Object Name | ForEach-Object {
    $name = $_.BaseName
    $expectedFile = Join-Path $root ("tests/expected/{0}.out" -f $name)

    & $exe $_.FullName --check *> $null
    if ($LASTEXITCODE -ne 0) {
        Write-Host "FAIL [check] $($_.Name)"
        $failed = $true
        return
    }

    $interp = Run-And-Capture $_.FullName '--run-interpreter'
    if ($interp.code -ne 0) {
        Write-Host "FAIL [interpreter] $($_.Name)"
        $failed = $true
        return
    }

    $native = Run-And-Capture $_.FullName '--run'
    if ($native.code -ne 0) {
        Write-Host "FAIL [codegen-run] $($_.Name)"
        $failed = $true
        return
    }

    $expected = Normalize-Text (Get-Content $expectedFile -Raw)
    $gotInterp = Normalize-Text $interp.output
    $gotNative = Normalize-Text $native.output

    if ($gotInterp -ne $expected) {
        Write-Host "FAIL [expected-vs-interpreter] $($_.Name)"
        Write-Host "Expected:`n$expected"
        Write-Host "Got:`n$gotInterp"
        $failed = $true
        return
    }

    if ($gotNative -ne $expected) {
        Write-Host "FAIL [expected-vs-native] $($_.Name)"
        Write-Host "Expected:`n$expected"
        Write-Host "Got:`n$gotNative"
        $failed = $true
        return
    }

    if ($gotNative -ne $gotInterp) {
        Write-Host "FAIL [parity] $($_.Name)"
        $failed = $true
        return
    }

    Write-Host "PASS $($_.Name)"
}

Write-Host 'Running ERR tests...'
Get-ChildItem tests/err/*.hs | Sort-Object Name | ForEach-Object {
    $cmd = '"' + $exe + '" "' + $_.FullName + '" --check >nul 2>nul'
    cmd /c $cmd
    if ($LASTEXITCODE -eq 0) {
        Write-Host "FAIL [expected error] $($_.Name)"
        $failed = $true
    } else {
        Write-Host "PASS $($_.Name)"
    }
}

Write-Host 'Running semantic error tests...'
Get-ChildItem tests/semerr/*.hs | Sort-Object Name | ForEach-Object {
    $cmd = '"' + $exe + '" "' + $_.FullName + '" --run-interpreter >nul 2>nul'
    cmd /c $cmd
    if ($LASTEXITCODE -eq 0) {
        Write-Host "FAIL [expected semantic error] $($_.Name)"
        $failed = $true
    } else {
        Write-Host "PASS $($_.Name)"
    }
}

if ($failed) {
    Write-Host 'One or more tests failed.'
    exit 1
}

Write-Host 'All tests passed.'
exit 0
