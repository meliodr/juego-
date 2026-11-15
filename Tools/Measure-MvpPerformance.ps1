[CmdletBinding()]
param(
    [Parameter(Mandatory, Position = 0)]
    [string[]]$CsvPath,
    [switch]$ByPhase,
    [string]$InitialPhase = 'BeforeFirstMarker',
    [ValidateRange(0, 900)]
    [double]$MaximumSeconds = 0
)

$ErrorActionPreference = 'Stop'
$Culture = [System.Globalization.CultureInfo]::InvariantCulture
$NumberStyle = [System.Globalization.NumberStyles]::Float

function Get-TimingSummary {
    param(
        [string]$Name,
        [System.Collections.Generic.List[double]]$Samples
    )
    if ($Samples.Count -eq 0) { return }
    $Samples.Sort()
    $Mean = ($Samples | Measure-Object -Average).Average
    $LastIndex = $Samples.Count - 1
    [pscustomobject]@{
        Stat = $Name
        Samples = $Samples.Count
        AverageMs = [math]::Round($Mean, 3)
        P95Ms = [math]::Round($Samples[[math]::Ceiling($Samples.Count * 0.95) - 1], 3)
        P99Ms = [math]::Round($Samples[[math]::Ceiling($Samples.Count * 0.99) - 1], 3)
        MaxMs = [math]::Round($Samples[$LastIndex], 3)
        Over33Ms = @($Samples | Where-Object { $_ -gt (1000.0 / 30) }).Count
        Over50Ms = @($Samples | Where-Object { $_ -gt 50 }).Count
        AverageFps = if ($Name -ieq 'FrameTime') { [math]::Round(1000.0 / $Mean, 2) }
    }
}

foreach ($Path in $CsvPath) {
    $File = Get-Item -LiteralPath $Path
    $Groups = [ordered]@{ All = @{} }
    $ActiveGroups = @('All')
    $TimingNames = $null
    $ElapsedMs = 0.0
    if ($ByPhase -and $InitialPhase -and $InitialPhase -ine 'All') {
        $Groups[$InitialPhase] = @{}
        $ActiveGroups += $InitialPhase
    }
    Import-Csv -LiteralPath $File.FullName | ForEach-Object {
        if ($MaximumSeconds -gt 0 -and $ElapsedMs -ge $MaximumSeconds * 1000.0) { return }
        if ($null -eq $TimingNames) {
            $TimingNames = @($_.PSObject.Properties.Name -match '^(FrameTime|GameThreadTime|RenderThreadTime|RHIThreadTime|GPUTime|GPU(Frame)?TimeMs)$')
        }
        if ($ByPhase -and $_.Events -match 'IB:(\w+)') {
            $Phase = $Matches[1]
            if (!$Groups.Contains($Phase)) {
                $Groups[$Phase] = @{}
            }
            $ActiveGroups = @('All', $Phase)
        }
        foreach ($Timing in $TimingNames) {
            $Number = 0.0
            if (![double]::TryParse($_.$Timing, $NumberStyle, $Culture, [ref]$Number) -or $Number -le 0.0 -or
                [double]::IsInfinity($Number) -or [double]::IsNaN($Number)) {
                continue
            }
            if ($Timing -ieq 'FrameTime') { $ElapsedMs += $Number }
            foreach ($Group in $ActiveGroups) {
                $Stats = $Groups[$Group]
                if (!$Stats.ContainsKey($Timing)) {
                    $Stats[$Timing] = [System.Collections.Generic.List[double]]::new()
                }
                $Stats[$Timing].Add($Number)
            }
        }
    }
    if (!$Groups.All.ContainsKey('FrameTime')) {
        throw "No positive FrameTime samples in $($File.FullName). Supply an uncompressed Unreal CSV profiler capture."
    }
    foreach ($Group in $Groups.Keys) {
        $Stats = $Groups[$Group]
        if (!$Stats.ContainsKey('FrameTime')) { continue }
        $Columns = @(@{Name = 'Capture'; Expression = { $File.Name }}, @{Name = 'Phase'; Expression = { $Group }}, '*')
        foreach ($Stat in @('FrameTime') + @($Stats.Keys | Where-Object { $_ -ine 'FrameTime' } | Sort-Object)) {
            Get-TimingSummary $Stat $Stats[$Stat] | Select-Object -Property $Columns
        }
    }
}
