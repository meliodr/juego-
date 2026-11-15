param(
    [ValidateSet('Build','Author','RuntimeTests')][string]$Mode = 'Build',
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [switch]$ExclusiveTurnConfirmed
)
$ErrorActionPreference = 'Stop'
if (!$ExclusiveTurnConfirmed) { throw 'First coordinate exclusive Editor/Build/Cook ownership with the other sessions; then pass -ExclusiveTurnConfirmed.' }
$busy = Get-Process UnrealEditor,UnrealEditor-Cmd,ShaderCompileWorker,UnrealBuildTool -ErrorAction SilentlyContinue
if ($busy) { throw ('Unreal tools already running: ' + (($busy | ForEach-Object { "$($_.ProcessName):$($_.Id)" }) -join ', ')) }
$mutex = [Threading.Mutex]::new($false, 'Local\ImpactBeyond_UnrealExclusive')
if (!$mutex.WaitOne(0)) { $mutex.Dispose(); throw 'The cooperating Unreal tool lock is owned by another session.' }
try {
    $project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../ImpactBeyond/ImpactBeyond.uproject')).Path
    $projectRoot = Split-Path -Parent $project
    $evidence = Join-Path $projectRoot 'Saved/IB_S3_QA'
    $null = New-Item -ItemType Directory -Path $evidence -Force
    if ($Mode -eq 'Build') {
        & (Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat') ImpactBeyondEditor Win64 Development $project -WaitMutex -NoHotReload -NoHotReloadFromIDE -NoUBA -MaxParallelActions=2 2>&1 |
            Tee-Object -FilePath (Join-Path $evidence 'build.log')
        if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)." }
    } else {
        $editor = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
        $log = Join-Path $evidence ($Mode + '-' + [guid]::NewGuid().ToString('N') + '.log')
        $args = @(('"{0}"' -f $project),'-unattended','-nosplash','-nosound','-nullrhi','-NoLiveCoding',('-abslog="{0}"' -f $log))
        if ($Mode -eq 'Author') {
            $author = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot 'IBS3_AuthorLab.py')).Path
            # Native engine Python/editor scripting plugins for this process only; .uproject is untouched.
            $args += '-EnablePlugins=PythonScriptPlugin,EditorScriptingUtilities'
            $args += ('-ExecutePythonScript="{0}"' -f $author)
        } else {
            $args += '/Game/IB_S3/Lvl_IB_S3_PlanetLab'
            $args += '-game'
            $args += '-ExecCmds="Automation RunTests ImpactBeyond.S3"'
            $args += '-TestExit="Automation Test Queue Empty"'
            $args += ('-ReportExportPath="{0}"' -f (Join-Path $evidence 'Automation'))
        }
        $process = Start-Process -FilePath $editor -ArgumentList $args -WindowStyle Hidden -PassThru
        $watch = [Diagnostics.Stopwatch]::StartNew()
        while (!$process.WaitForExit(1000)) {
            if ($watch.Elapsed.TotalMinutes -gt 15) { throw "Own Unreal process $($process.Id) still running; inspect it before releasing the editor slot." }
        }
        if ($process.ExitCode -ne 0) { throw "Unreal failed ($($process.ExitCode)); see $log" }
        if ($Mode -eq 'Author') {
            if (!(Select-String -LiteralPath $log -SimpleMatch -Quiet 'IB_S3 authoring complete')) { throw "Author script did not report success: $log" }
        } else {
            foreach ($test in @('ImpactBeyond.S3.Manifest','ImpactBeyond.S3.RuntimeResidency')) {
                $pattern = 'Test Completed\. Result=\{Success\}.*Path=\{' + [regex]::Escape($test) + '\}'
                if (!(Select-String -LiteralPath $log -Pattern $pattern -Quiet)) { throw "Missing PASS for $test in $log" }
            }
        }
        Write-Host "Evidence: $log"
    }
} finally {
    $mutex.ReleaseMutex()
    $mutex.Dispose()
}
