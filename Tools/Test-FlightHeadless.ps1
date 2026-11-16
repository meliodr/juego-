param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [switch]$Circuit
)

$ErrorActionPreference = 'Stop'
$Project = (Resolve-Path (Join-Path $PSScriptRoot '../ImpactBeyond/ImpactBeyond.uproject')).Path
$Build = Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat'
$Editor = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$LogDir = Join-Path (Split-Path $Project) 'Saved/Logs'
New-Item -ItemType Directory -Force $LogDir | Out-Null
$TestName = if ($Circuit) { 'ImpactBeyond.Circuit.Headless' } else { 'ImpactBeyond.Flight.Headless' }
$Map = if ($Circuit) { '/Game/IB_C00/Lvl_IB_C00' } else { '/Game/ThirdPerson/Lvl_ThirdPerson' }
$Log = Join-Path $LogDir ('{0}-{1}.log' -f $TestName, ([guid]::NewGuid().ToString('N')))

& $Build ImpactBeyondEditor Win64 Development $Project -WaitMutex -NoHotReload -NoHotReloadFromIDE -NoUBA
if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)." }

$Arguments = @(
    ('"{0}"' -f $Project)
    $Map
    '-game -nullrhi -unattended -nosplash -nosound -nopause -NoLiveCoding'
    '-UseFixedTimeStep -FPS=60'
    ('-ExecCmds="Automation RunTests {0}"' -f $TestName)
    '-TestExit="Automation Test Queue Empty"'
    ('-abslog="{0}"' -f $Log)
)
Write-Host "Unreal log: $Log"
$Process = Start-Process -FilePath $Editor -ArgumentList $Arguments -WindowStyle Hidden -PassThru
if (!$Process.WaitForExit(300000)) {
    Stop-Process -InputObject $Process
    throw 'Headless run timed out after five minutes.'
}
if (!(Test-Path -LiteralPath $Log)) { throw 'Unreal did not produce a log.' }
Select-String -LiteralPath $Log -Pattern 'LogAutomationController:|Flight cap|After 20 frames|Wall \(' |
    Select-Object -Last 30 | ForEach-Object { $_.Line }
$Success = 'Test Completed\. Result=\{Success\}.*Path=\{' + [regex]::Escape($TestName) + '\}'
if ($Process.ExitCode -ne 0 -or !(Select-String -LiteralPath $Log -Quiet -Pattern $Success)) {
    throw "$TestName failed or did not complete (Unreal exit $($Process.ExitCode))."
}
