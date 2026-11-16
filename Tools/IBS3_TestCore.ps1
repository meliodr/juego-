param([string]$CompilerRoot = 'C:\Program Files\Microsoft Visual Studio\18\Community', [string]$OutputDirectory,
      [ValidateSet('Core','Review')][string]$Suite = 'Core')
$ErrorActionPreference = 'Stop'
if (!$OutputDirectory) { $OutputDirectory = Join-Path ([IO.Path]::GetTempPath()) ('IBS3_QA_' + [guid]::NewGuid().ToString('N')) }
$null = New-Item -ItemType Directory -Force -Path $OutputDirectory
$outputRoot = (Resolve-Path -LiteralPath $OutputDirectory).Path
$vcvars = Join-Path $CompilerRoot 'VC\Auxiliary\Build\vcvars64.bat'
$source = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot ('IBS3_' + $Suite + 'Tests.cpp'))).Path
if (!(Test-Path -LiteralPath $vcvars)) { throw 'MSVC environment not found; pass -CompilerRoot. No installation attempted.' }
# Use one temporary build script with literal, quoted paths; this performs no deletion/moving.
$batch = Join-Path $outputRoot 'compile-core.cmd'
@"
@echo off
call "$vcvars" >nul
if errorlevel 1 exit /b 1
cl /nologo /std:c++17 /EHsc /W4 /WX /O2 "$source" /Fe:"$outputRoot\IBS3_CoreTests.exe" /Fo:"$outputRoot\IBS3_CoreTests.obj"
if errorlevel 1 exit /b 1
"$outputRoot\IBS3_CoreTests.exe"
"@ | Set-Content -LiteralPath $batch -Encoding ascii
& $env:ComSpec /d /c $batch 2>&1 | Tee-Object -FilePath (Join-Path $outputRoot 'core-test.log')
if ($LASTEXITCODE -ne 0) { throw "Native geometry test failed ($LASTEXITCODE); see $outputRoot" }
Write-Host "Evidence: $outputRoot"
