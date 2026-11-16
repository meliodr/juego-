param(
    [Parameter(Mandatory=$true)][string]$S4Root,
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [string]$Toolchain = 'C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat'
)
$ErrorActionPreference = 'Stop'
# Source boundary checks complement native policy tests; these do NOT execute UObject binding.
$bridge = Get-Content -Raw -LiteralPath (Join-Path $PSScriptRoot '..\ImpactContractUE.h')
if ($bridge.Contains('Target->Receive(Hit)')) { throw 'Bridge recurses through native ingress instead of the bound endpoint.' }
foreach ($required in @('TWeakObjectPtr<UObject> BindingOwner;', 'BindingOwner.IsValid()', 'Target->HasConsumerBinding()',
    'Target->ConsumeBoundHit(BindingOwner.Get(), Hit, E.PhysicsResolved)',
    'Record ReceiveS4(const FIBS4Hit& Input, const S4Identity& Identity)', 'E.PhysicsResolved = Identity.bPhysicsResolved;')) {
    if (-not $bridge.Contains($required)) { throw "Missing bridge boundary: $required" }
}
if (-not (Test-Path -LiteralPath $Toolchain)) { throw 'MSVC not present. Nothing will be installed.' }
$s4Source = Join-Path $S4Root 'ImpactBeyond\Source\ImpactBeyond\IB_S4'
$files = @('IBS4Policy.h','IBS4ResponseComponent.h','IBS4ResponseComponent.cpp')
$before = @{}
foreach ($file in $files) { $before[$file] = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $s4Source $file)).Hash }
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$source = Join-Path $PSScriptRoot 'ImpactContractNative.cpp'
$exe = Join-Path $OutputDirectory 'ImpactContractNative.exe'
$obj = Join-Path $OutputDirectory 'ImpactContractNative.obj'
$command = 'call "{0}" >nul && cl /nologo /std:c++17 /EHsc /W4 /WX /Od /DIB_IMPACT_NATIVE_TEST /I"{1}" "{2}" /Fo"{3}" /Fe"{4}"' -f $Toolchain,$s4Source,$source,$obj,$exe
$compile = & $env:ComSpec /d /s /c $command 2>&1
$compile | Tee-Object -FilePath (Join-Path $OutputDirectory 'compile.log') | Write-Output
if ($LASTEXITCODE -ne 0) { throw "Native compile failed: $LASTEXITCODE" }
$run = & $exe 2>&1
$run | Tee-Object -FilePath (Join-Path $OutputDirectory 'run.log') | Write-Output
if ($LASTEXITCODE -ne 0) { throw "Native tests failed: $LASTEXITCODE" }
foreach ($file in $files) {
    $after = (Get-FileHash -Algorithm SHA256 -LiteralPath (Join-Path $s4Source $file)).Hash
    if ($before[$file] -ne $after) { throw "S4 source changed during run: $file" }
}
$report = [ordered]@{
    utc = [DateTime]::UtcNow.ToString('o')
    test = 'ImpactContractNative'
    result = ($run -join "`n")
    compileFlags = '/std:c++17 /EHsc /W4 /WX /Od /DIB_IMPACT_NATIVE_TEST'
    bridgeSourceBoundary = 'PASS: bound endpoint/owner/fail-closed guard/physics flag; static checks only'
    s4SourceSha256 = $before
    s4SourceUnchangedDuringRun = $true
    unrealCompiled = $false
    unrealExecuted = $false
    visualPhysicsValidated = $false
}
$report | ConvertTo-Json -Depth 5 | Set-Content -Encoding UTF8 -LiteralPath (Join-Path $OutputDirectory 'result.json')
