param(
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8'
)

$ErrorActionPreference = 'Stop'
$Project = (Resolve-Path (Join-Path $PSScriptRoot '../ImpactBeyond/ImpactBeyond.uproject')).Path
$Build = Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat'
$Editor = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
$LogDir = Join-Path (Split-Path $Project) 'Saved/Logs'
New-Item -ItemType Directory -Force $LogDir | Out-Null
$Log = Join-Path $LogDir ('IB-F01-{0}.log' -f ([guid]::NewGuid().ToString('N')))

& $Build ImpactBeyondEditor Win64 Development $Project -WaitMutex -NoHotReload -NoHotReloadFromIDE -NoUBA
if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)." }

$Arguments = @(
    ('"{0}"' -f $Project)
    '/Game/ThirdPerson/Lvl_ThirdPerson'
    '-game -nullrhi -unattended -nosplash -nosound -nopause -NoLiveCoding'
    '-UseFixedTimeStep -FPS=60'
    '-ExecCmds="Automation RunTests ImpactBeyond.Flight.Headless"'
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
if ($Process.ExitCode -ne 0 -or !(Select-String -LiteralPath $Log -Quiet -Pattern 'Test Completed\. Result=\{Success\}.*Path=\{ImpactBeyond\.Flight\.Headless\}')) {
    throw "Flight test failed or did not complete (Unreal exit $($Process.ExitCode))."
}
