param(
    [ValidateSet('Native', 'Build', 'Author', 'Automation', 'DX12')]
    [string]$Phase = 'Native',
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [switch]$ExclusiveEditorSlotConfirmed
)
$ErrorActionPreference = 'Stop'
$Repo = Split-Path -Parent $PSScriptRoot
if ($Phase -eq 'Native') {
    & python (Join-Path $PSScriptRoot 'IBS4_Check.py')
    exit $LASTEXITCODE
}
if (-not $ExclusiveEditorSlotConfirmed) {
    throw 'Coordinate a real exclusive Editor/build slot with the other sessions first. This flag is not a cross-session lock.'
}
$Busy = Get-Process UnrealEditor*,UnrealBuildTool*,ShaderCompileWorker*,UnrealPak*,AutomationTool* -ErrorAction SilentlyContinue
if ($Busy) { throw ('Another Unreal operation is running: ' + (($Busy | Select-Object -ExpandProperty ProcessName) -join ', ')) }
$Project = Join-Path $Repo 'ImpactBeyond\ImpactBeyond.uproject'
$Editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$Build = Join-Path $EngineRoot 'Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path -LiteralPath $Project)) { throw 'Project descriptor is missing in this checkout.' }
if ($Phase -eq 'Build') {
    & $Build ImpactBeyondEditor Win64 Development "-Project=$Project" -WaitMutex -NoHotReloadFromIDE -MaxParallelActions=2
} elseif ($Phase -eq 'Author') {
    $Script = Join-Path $PSScriptRoot 'IBS4_AuthorLab.py'
    & $Editor $Project "-ExecutePythonScript=$Script" -unattended -nosplash -NullRHI -log
} elseif ($Phase -eq 'Automation') {
    & $Editor $Project '/Game/IB_S4/Lvl_IB_S4_Response' -game -NullRHI -unattended -nosplash '-ExecCmds=Automation RunTests ImpactBeyond.S4' '-TestExit=Automation Test Queue Empty' -log
} else {
    # Render/audio capture under the exclusive slot. This is not a Steam/DualSense
    # acceptance session; the packaged user-facing run must follow the repo's Steam gate.
    & $Editor $Project '/Game/IB_S4/Lvl_IB_S4_Response' -game -d3d12 -ResX=1920 -ResY=1080 -windowed -log
}
exit $LASTEXITCODE
