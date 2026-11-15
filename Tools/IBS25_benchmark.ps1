[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$Editor,
    [Parameter(Mandatory)][string]$Project,
    [ValidateSet('A','B')][string]$Variant = 'A',
    [switch]$EditorTurnConfirmed,
    [switch]$Launch
)
$ErrorActionPreference = 'Stop'
# A process check is NOT a reservation. Obtain a turn from the coordinator first.
# Without -Launch this script emits a reviewable command and changes nothing.
$editorPath = (Get-Item -LiteralPath $Editor).FullName
$projectPath = (Get-Item -LiteralPath $Project).FullName
if ([IO.Path]::GetFileName($editorPath) -ne 'UnrealEditor.exe') { throw 'Use the installed UnrealEditor.exe' }
if ([IO.Path]::GetExtension($projectPath) -ne '.uproject') { throw 'Expected .uproject' }
if ($editorPath.Contains('"') -or $projectPath.Contains('"')) { throw 'Invalid argument quoting' }
$scale = if ($Variant -eq 'A') { 100 } else { 83 }
$commands = @(
    'r.VSync 0', 't.MaxFPS 0', 'r.AntiAliasingMethod 4',
    'r.EyeAdaptation.CachedLightingPreExposure 8',
    'r.DynamicGlobalIlluminationMethod 1', 'r.ReflectionMethod 1',
    'r.Shadow.Virtual.Enable 0', 'sg.ShadowQuality 2',
    'sg.GlobalIlluminationQuality 2', 'sg.ReflectionQuality 2',
    'r.Streaming.PoolSize 2500', "r.ScreenPercentage $scale"
) -join ','
$arguments = '"{0}" /Game/IB_S25/Lvl_IB_S25_RealismLab -game -dx12 -windowed -ResX=1920 -ResY=1080 -log -ExecCmds="{1}"' -f $projectPath,$commands
[pscustomobject]@{Variant=$Variant;Editor=$editorPath;Arguments=$arguments;Audio='ON (no -nosound)';Render='visible';Status='COMMAND_ONLY_NOT_MEASURED'}
if (!$Launch) { return }
if (!$EditorTurnConfirmed) { throw 'PENDING: explicit coordinated Unreal turn required' }
$expectedProject = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\ImpactBeyond\ImpactBeyond.uproject'))
if ($projectPath -ne $expectedProject) { throw 'Use this isolated worktree project; do not launch a foreign checkout' }
$projectRoot = Split-Path $projectPath -Parent
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Content\IB_S25\Lvl_IB_S25_RealismLab.umap'))) {
    throw 'PENDING: lab map has not been authored'
}
if (!(Test-Path -LiteralPath (Join-Path $projectRoot 'Binaries\Win64\UnrealEditor-ImpactBeyond.dll'))) {
    throw 'PENDING: compatible editor module missing; no automatic compilation'
}
if (Get-Process -Name UnrealEditor,UnrealEditor-Cmd,ShaderCompileWorker,UnrealBuildTool,AutomationTool -ErrorAction SilentlyContinue) {
    throw 'PENDING: Unreal/editor/build work is already running; do not terminate it'
}
# Cooperative additional guard; it cannot coordinate sessions that do not use this mutex.
$mutex = [Threading.Mutex]::new($false,'Local\IMPACT_Unreal_HeavyWork')
$acquired = $false
try {
    $acquired = $mutex.WaitOne(0)
    if (!$acquired) { throw 'PENDING: another coordinated Unreal job holds the mutex' }
    # Hidden helper startup; the game creates its requested visible render window.
    $process = Start-Process -FilePath $editorPath -ArgumentList $arguments -WindowStyle Hidden -PassThru
    Write-Output "Started PID $($process.Id). Use the fixed camera protocol in docs/IB_S25_REALISM_LAB.md."
    $runRoot = Join-Path $PSScriptRoot ('IBS25_evidence\AB_' + $Variant + '_' + [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfff'))
    New-Item -ItemType Directory -Path $runRoot -ErrorAction Stop | Out-Null
    $gpu = Get-Command nvidia-smi.exe -ErrorAction SilentlyContinue
    if ($gpu) {
        'timestamp,gpu_name,memory_used_mib,memory_total_mib,gpu_utilization_percent' | Set-Content -LiteralPath (Join-Path $runRoot 'gpu.csv')
    }
    while (!$process.HasExited) {
        if ($gpu) {
            & $gpu.Source -i 0 --query-gpu=timestamp,name,memory.used,memory.total,utilization.gpu --format=csv,noheader,nounits |
                Add-Content -LiteralPath (Join-Path $runRoot 'gpu.csv')
        }
        Start-Sleep -Milliseconds 500
        $process.Refresh()
    }
    $process.WaitForExit()
    [pscustomobject]@{Variant=$Variant;Scale=$scale;Pid=$process.Id;ExitCode=$process.ExitCode;
        MemoryScope='WHOLE_GPU, includes desktop/other processes; not per-process VRAM';
        GPUAvailable=[bool]$gpu;DX12='VERIFY_LOG';Audio='VERIFY_DEVICE_AND_AUDIBILITY';
        Render='VERIFY_VISIBLE';Csv='Collect manually with csvprofile start/stop per camera'} |
        ConvertTo-Json | Set-Content -LiteralPath (Join-Path $runRoot 'session.json')
    if ($process.ExitCode -ne 0) { throw "Unreal exited with code $($process.ExitCode)" }
} finally {
    if ($acquired) { $mutex.ReleaseMutex() }
    $mutex.Dispose()
}
