param(
    [ValidateSet('Build','Author','RepairLighting','RuntimeTests','Contacts','FlightReentry','Circumnavigation','RestartWrite','RestartVerify','VisualEvidence')][string]$Mode = 'Build',
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$QASaveSlot,
    [switch]$ExclusiveTurnConfirmed
)
$ErrorActionPreference = 'Stop'
if (!$ExclusiveTurnConfirmed) { throw 'First coordinate exclusive Editor/Build/Cook ownership with the other sessions; then pass -ExclusiveTurnConfirmed.' }
$busy = Get-Process UnrealEditor,UnrealEditor-Cmd,ShaderCompileWorker,UnrealBuildTool -ErrorAction SilentlyContinue
if ($busy) { throw ('Unreal tools already running: ' + (($busy | ForEach-Object { "$($_.ProcessName):$($_.Id)" }) -join ', ')) }
$builders = Get-CimInstance Win32_Process | Where-Object { $_.Name -match 'dotnet|AutomationTool|MSBuild|^cl.exe$|^link.exe$' }
if ($builders) { throw ('Build processes require owner review before launch: ' + (($builders | ForEach-Object { "$($_.Name):$($_.ProcessId) $($_.CommandLine)" }) -join ', ')) }
if ($Mode -in @('RestartWrite','RestartVerify') -and $QASaveSlot -notmatch '^IB_S3_QA_Restart_[0-9a-fA-F]{32}$') { throw 'Restart modes require a GUID-named -QASaveSlot.' }
$mutex = [Threading.Mutex]::new($false, 'Local\ImpactBeyond_UnrealExclusive')
if (!$mutex.WaitOne(0)) { $mutex.Dispose(); throw 'The cooperating Unreal tool lock is owned by another session.' }
try {
    $project = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '../ImpactBeyond/ImpactBeyond.uproject')).Path
    $projectRoot = Split-Path -Parent $project
    $evidence = Join-Path $projectRoot 'Saved/IB_S3_QA'
    $null = New-Item -ItemType Directory -Path $evidence -Force
    if ($Mode -eq 'Build') {
        # The immutable S2 base has duplicate anonymous-namespace names in separate translation units.
        # Compile separately rather than editing shared S2 sources to accommodate unity builds.
        & (Join-Path $EngineRoot 'Engine/Build/BatchFiles/Build.bat') ImpactBeyondEditor Win64 Development $project -WaitMutex -NoHotReload -NoHotReloadFromIDE -DisableUnity -NoUBA -MaxParallelActions=2 2>&1 |
            Tee-Object -FilePath (Join-Path $evidence 'build.log')
        if ($LASTEXITCODE -ne 0) { throw "Build failed ($LASTEXITCODE)." }
    } else {
        $editor = Join-Path $EngineRoot 'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
        $log = Join-Path $evidence ($Mode + '-' + [guid]::NewGuid().ToString('N') + '.log')
        # -nowrite prevents first-launch editor modules from rewriting shared DefaultEngine.ini.
        $args = @(('"{0}"' -f $project),'-unattended','-nosplash','-nosound','-NoLiveCoding','-nowrite',('-abslog="{0}"' -f $log))
        if ($Mode -eq 'VisualEvidence') { $args += @('-d3d12','-RenderOffscreen','-windowed','-ResX=1920','-ResY=1080','-ForceRes') }
        else { $args += '-nullrhi' }
        if ($Mode -in @('Author','RepairLighting')) {
            $scriptName = if ($Mode -eq 'Author') { 'IBS3_AuthorLab.py' } else { 'IBS3_RepairLighting.py' }
            $author = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot $scriptName)).Path
            # Native engine Python/editor scripting plugins for this process only; .uproject is untouched.
            $args += '-EnablePlugins=PythonScriptPlugin,EditorScriptingUtilities'
            $args += ('-ExecutePythonScript="{0}"' -f $author)
        } else {
            $tests = if ($Mode -eq 'RuntimeTests') { @('Manifest','RuntimeResidency','SaveValidation') } else { @($Mode) }
            $fullTests = @($tests | ForEach-Object { 'ImpactBeyond.S3.' + $_ })
            $args += '/Game/IB_S3/Lvl_IB_S3_PlanetLab'
            $args += '-game'
            $args += ('-ExecCmds="t.MaxFPS 60,Automation RunTests {0}"' -f ($fullTests -join '+'))
            if ($QASaveSlot) { $args += ('-IBS3QASaveSlot=' + $QASaveSlot) }
            $args += '-TestExit="Automation Test Queue Empty"'
            $args += ('-ReportExportPath="{0}"' -f (Join-Path $evidence ('Automation-' + [IO.Path]::GetFileNameWithoutExtension($log))))
        }
        $process = Start-Process -FilePath $editor -ArgumentList $args -WindowStyle Hidden -PassThru
        Write-Host "Own process PID=$($process.Id) Mode=$Mode Log=$log"
        $watch = [Diagnostics.Stopwatch]::StartNew()
        $nextNoticeMinutes = 15
        while (!$process.WaitForExit(1000)) {
            if ($watch.Elapsed.TotalMinutes -gt $nextNoticeMinutes) {
                Write-Warning "Own Unreal process $($process.Id) still running; retaining exclusive mutex. Inspect $log."
                $nextNoticeMinutes += 5
            }
        }
        if ($process.ExitCode -ne 0) { throw "Unreal failed ($($process.ExitCode)); see $log" }
        if ($Mode -in @('Author','RepairLighting')) {
            $marker = if ($Mode -eq 'Author') { 'IB_S3 authoring complete' } else { 'IB_S3 lighting repair complete' }
            if (!(Select-String -LiteralPath $log -SimpleMatch -Quiet $marker)) { throw "Editor script did not report success: $log" }
        } else {
            foreach ($test in $fullTests) {
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
