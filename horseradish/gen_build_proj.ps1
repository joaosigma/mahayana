param (
    [Parameter(Mandatory=$true, Position=0)]
    [ValidateNotNullOrEmpty()]
    [ValidateSet('vs-2026', 'ninja')]
    [String] $buildTarget = "vs-2026",

    [Parameter(Mandatory=$false)]
    [ValidateNotNullOrEmpty()]
    [ValidateSet('debug', 'release')]
    [String] $buildMode = "release",

    [Parameter(Mandatory=$false)]
    [switch] $buildTests = $false
)

$ErrorActionPreference = 'Stop'

function Set-VSEnv {
    if ($null -ne $env:VCToolsVersion -And $null -ne $env:VCToolsInstallDir) {
        Write-Host "Skipping setting up Visual Studio env."
        return
    }

    Write-Host "Setting up Visual Studio env..."

    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

    $vcvarspath = &$vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath

    cmd.exe /c "call `"$vcvarspath\VC\Auxiliary\Build\vcvars64.bat`" && set > %temp%\vcvars.txt"

    Get-Content "$env:temp\vcvars.txt" | Foreach-Object {
        if ($_ -match "^(.*?)=(.*)$") {
            Set-Content "env:\$($matches[1])" $matches[2]
        }
    }

    Remove-Item "$env:temp\vcvars.txt"
}

function Get-VSExecPath {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    return &$vswhere -latest -property productPath
}

switch ($buildTarget) {
    "ninja" { Write-Host ">>>>> Generating Horseradish project (Ninja in '$buildMode')" }
    "vs-2026" { Write-Host ">>>>> Generating Horseradish project (VStudio 2026)" }
    default { Write-Host ">>>>> Generating Horseradish project (unknown builder)" }
}
Write-Host ">>>>> (good luck)"
Write-Host ""

# prepare stuff related to VCPKG

if (-not (Test-Path -Path "build-tools/vcpkg")) {
    $env:PATH = "$env:VCPKG_DISABLE_METRICS;$env:PATH"

    Write-Host "Downloading and setting up vcpkg..."

    New-Item -Path "." -Name "build-tools/vcpkg" -ItemType Directory | Out-Null
    Push-Location -Path "build-tools/vcpkg"
    try {
        git clone --no-tags --depth 1 --shallow-submodules https://github.com/microsoft/vcpkg.git .
        .\bootstrap-vcpkg.bat
    } finally {
        Pop-Location
    }

    Write-Host ""
}

# prepare stuff related to the build target folder and utils (i.e.: download ninja if necessary)

if ($buildTarget -eq "ninja") {
    if (-not (Test-Path -Path "build-tools/ninja")) {
        Write-Host "Downloading Ninja..."

        New-Item -Path "." -Name "build-tools/ninja" -ItemType Directory | Out-Null

        Push-Location -Path "build-tools/ninja"
        try {
            Start-BitsTransfer -Source "https://github.com/ninja-build/ninja/releases/latest/download/ninja-win.zip"
            Expand-Archive -Path "ninja-win.zip" -DestinationPath "./"
            Remove-Item -Path "./ninja-win.zip"
        } finally {
            Pop-Location
        }
    }

    $ninjaPath = Resolve-Path -Path "./build-tools/ninja"
    if (-not (Test-Path -Path $ninjaPath)) {
        Write-Error "Unable to figure out the path for ninja executable."
    }

    Write-Host ""
}

# now we can run cmake to generate the build solution / files


if ($buildTarget -eq "ninja") {

    Set-VSEnv

    $preset = "ninja-$buildMode"

    cmake --preset $preset
    if ($LASTEXITCODE -ne 0) { return; }

    if ($buildTests) {
        Write-Host "Building '$preset' and running tests..."
        Write-Host ""

        cmake --build --preset $preset
        if ($LASTEXITCODE -ne 0) { return; }

        ctest --preset $preset
    }

} else {

    if ($buildTarget -eq "vs-2026") {
        $preset = "vs-2026"
    } else {
        Write-Error "Unknown VisualStudio '$buildTarget' generator."
    }

    cmake --preset $preset
    if ($LASTEXITCODE -ne 0) { return; }

    if ($buildTests) {

        $presetTests = "$preset-$buildMode"

        Write-Host "Building '$presetTests' and running tests..."
        Write-Host ""

        cmake --build --preset $presetTests
        if ($LASTEXITCODE -ne 0) { return; }

        ctest --preset $presetTests

    } else {

        $idePath = Get-VSExecPath
        &$idePath "build-$preset\mustard.slnx"

    }

}

Write-Host ">>>>> done"
