param([string]$OutputDirectory = (Join-Path $env:TEMP 'IBS35-policy-tests'))
$ErrorActionPreference = 'Stop'
$toolchain = 'C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path -LiteralPath $toolchain)) { throw 'MSVC environment not found; supply the local toolchain manually. Nothing will be installed.' }
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$source = Join-Path $PSScriptRoot 'IBS35_TestPolicy.cpp'
$program = Join-Path $OutputDirectory 'IBS35_TestPolicy.exe'
$obj = Join-Path $OutputDirectory 'IBS35_TestPolicy.obj'
# One small native translation unit; no UBT, editor, cook or global environment change.
$command = 'call "{0}" >nul && cl /nologo /std:c++17 /EHsc /W4 /WX /Od "{1}" /Fo"{2}" /Fe"{3}"' -f $toolchain,$source,$obj,$program
& $env:ComSpec /d /s /c $command
if ($LASTEXITCODE -ne 0) { throw "Policy compile failed: $LASTEXITCODE" }
& $program
if ($LASTEXITCODE -ne 0) { throw "Policy tests failed: $LASTEXITCODE" }
