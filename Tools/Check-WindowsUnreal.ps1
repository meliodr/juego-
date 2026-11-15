#requires -Version 5.1
<#
.SYNOPSIS
  Auditoria de solo lectura para IMPACT: BEYOND (Windows / Unreal 5.8).
.DESCRIPTION
  Consulta Windows, CPU, GPU, unidades, Unreal, Visual Studio/MSVC y Windows SDK.
  No instala, no modifica el registro, no cambia drivers, no hace peticiones de red
  y no escribe archivos. Imprime un resumen JSON para el ticket IB-W00.
.NOTES
  La deteccion de Unreal y de DX12 NO equivale a abrir el editor ni a ejecutar el juego.
  Evitar subir la salida completa a GitHub si incluye rutas de usuario.
#>
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Read-Safely {
    param(
        [scriptblock]$Operation,
        [object]$Fallback = $null
    )
    try { return (& $Operation) }
    catch { return $Fallback }
}

function File-Exists {
    param([string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) { return $false }
    return [bool](Test-Path -LiteralPath $Path -PathType Leaf)
}

function Folder-Exists {
    param([string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) { return $false }
    return [bool](Test-Path -LiteralPath $Path -PathType Container)
}

function GigaBytes {
    param([object]$Bytes)
    if ($null -eq $Bytes) { return $null }
    return [math]::Round(([double]$Bytes / 1GB), 1)
}

$os = Read-Safely { Get-CimInstance -ClassName Win32_OperatingSystem }
$cpu = @(Read-Safely { Get-CimInstance -ClassName Win32_Processor } @())
$gpus = @(Read-Safely { Get-CimInstance -ClassName Win32_VideoController } @())
$system = Read-Safely { Get-CimInstance -ClassName Win32_ComputerSystem }

# Registry/standard paths are hints; a portable/custom engine installation
# may be elsewhere. Check both registered builds and common launcher location.
$engineCandidates = @()
$defaultRoot = Join-Path $env:ProgramFiles 'Epic Games'
if (Folder-Exists $defaultRoot) {
    $engineCandidates += @(Get-ChildItem -LiteralPath $defaultRoot -Directory -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -like 'UE_*' } | Select-Object -ExpandProperty FullName)
}
$engineRegistry = 'HKLM:\SOFTWARE\EpicGames\Unreal Engine'
if (Test-Path -LiteralPath $engineRegistry) {
    foreach ($subKey in @(Get-ChildItem -LiteralPath $engineRegistry -ErrorAction SilentlyContinue)) {
        $properties = Read-Safely { Get-ItemProperty -LiteralPath $subKey.PSPath }
        if ($properties -and $properties.PSObject.Properties['InstalledDirectory']) {
            $engineCandidates += [string]$properties.InstalledDirectory
        }
    }
}
$engineBuildRegistry = 'HKCU:\SOFTWARE\Epic Games\Unreal Engine\Builds'
$buildProps = Read-Safely { Get-ItemProperty -LiteralPath $engineBuildRegistry }
if ($buildProps) {
    foreach ($p in @($buildProps.PSObject.Properties)) {
        if ($p.Name -like 'PS*') { continue }
        if ($p.Value -is [string] -and (Folder-Exists $p.Value)) {
            $engineCandidates += $p.Value
        }
    }
}
$engines = @(
    $engineCandidates | Sort-Object -Unique | ForEach-Object {
        $exe = Join-Path $_ 'Engine\Binaries\Win64\UnrealEditor.exe'
        if (File-Exists $exe) {
            [pscustomobject]@{ Root = $_; EditorExeFound = $true }
        }
    }
)

$vswhere = $null
$programFilesX86 = [Environment]::GetFolderPath('ProgramFilesX86')
if (-not [string]::IsNullOrEmpty($programFilesX86)) {
    $candidate = Join-Path $programFilesX86 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (File-Exists $candidate) { $vswhere = $candidate }
}
$vsInstallPath = $null
if ($vswhere) {
    $vsInstallPath = Read-Safely {
        (& $vswhere -latest -products '*' -property installationPath | Select-Object -First 1)
    }
}
$msvcVersions = @()
$clAvailable = $false
if ($vsInstallPath) {
    $msvcRoot = Join-Path $vsInstallPath 'VC\Tools\MSVC'
    if (Folder-Exists $msvcRoot) {
        $msvcVersions = @(
            Get-ChildItem -LiteralPath $msvcRoot -Directory -ErrorAction SilentlyContinue |
            Select-Object -ExpandProperty Name
        )
        foreach ($version in $msvcVersions) {
            $compiler = Join-Path $msvcRoot "$version\bin\Hostx64\x64\cl.exe"
            if (File-Exists $compiler) { $clAvailable = $true }
        }
    }
}

$sdkRoot = $null
$sdkRegistry = Read-Safely {
    Get-ItemProperty -LiteralPath 'HKLM:\SOFTWARE\Microsoft\Windows Kits\Installed Roots'
}
if ($sdkRegistry -and $sdkRegistry.PSObject.Properties['KitsRoot10']) {
    $sdkRoot = [string]$sdkRegistry.KitsRoot10
}
$sdkVersions = @()
if ($sdkRoot) {
    $includeRoot = Join-Path $sdkRoot 'Include'
    if (Folder-Exists $includeRoot) {
        $sdkVersions = @(
            Get-ChildItem -LiteralPath $includeRoot -Directory -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -match '^10\.0\.\d+\.\d+$' } |
            Select-Object -ExpandProperty Name
        )
    }
}

$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$repoDriveName = [System.IO.Path]::GetPathRoot($repoRoot).TrimEnd('\').TrimEnd(':')
$disk = Read-Safely { Get-PSDrive -Name $repoDriveName -PSProvider FileSystem }
$gitAvailable = $null -ne (Get-Command git -ErrorAction SilentlyContinue)
$gitChanges = $null
if ($gitAvailable -and (Folder-Exists (Join-Path $repoRoot '.git'))) {
    $gitChanges = Read-Safely {
        @(& git --no-optional-locks -C $repoRoot status --porcelain 2>$null).Count
    }
}

# Win32_VideoController.AdapterRAM is commonly capped/misreported for modern
# 8+ GB GPUs. Deliberately do not present it as an accurate VRAM measurement.
$result = [ordered]@{
    Project = 'IMPACT: BEYOND'
    CheckId = 'IB-W00'
    Status = 'INSPECTION_ONLY_NOT_A_BUILD_TEST'
    Windows = [ordered]@{
        Caption = if ($os) { $os.Caption } else { $null }
        Version = if ($os) { $os.Version } else { $null }
        BuildNumber = if ($os) { $os.BuildNumber } else { $null }
    }
    Cpu = @($cpu | ForEach-Object {
        [ordered]@{ Name = $_.Name; Cores = $_.NumberOfCores; Threads = $_.NumberOfLogicalProcessors }
    })
    MemoryGB = if ($system) { GigaBytes $system.TotalPhysicalMemory } else { $null }
    Gpus = @($gpus | ForEach-Object {
        [ordered]@{ Name = $_.Name; DriverVersion = $_.DriverVersion }
    })
    DirectX12DllDetected = File-Exists (Join-Path $env:WINDIR 'System32\d3d12.dll')
    UnrealEditorInstallationsFound = @($engines)
    VisualStudio = [ordered]@{
        VsWhereFound = $null -ne $vswhere
        InstallFound = $null -ne $vsInstallPath
        MsvcVersionFolders = @($msvcVersions)
        X64CompilerDetected = $clAvailable
    }
    WindowsSdkVersions = @($sdkVersions)
    Repository = [ordered]@{
        GitFound = $gitAvailable
        TrackedCheckoutPresent = Folder-Exists (Join-Path $repoRoot '.git')
        ChangedFileCount = $gitChanges
        DriveFreeGB = if ($disk) { GigaBytes $disk.Free } else { $null }
    }
    Warnings = @(
        'An UnrealEditor.exe file does not prove the editor launches.'
        'A DX12 DLL does not establish GPU DX12 feature support.'
        'No compiler, project or gameplay was built or run by this script.'
        'If installed in a nonstandard path, Unreal may not be detected.'
        'Review paths and device data before sharing this report.'
    )
}

$result | ConvertTo-Json -Depth 6
