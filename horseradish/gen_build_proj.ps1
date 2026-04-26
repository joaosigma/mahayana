param (
    [Parameter(Mandatory=$false)]
    [ValidateNotNullOrEmpty()]
    [ValidateSet('vs', 'ninja')]
    [String] $buildTool = "vs",

    [Parameter(Mandatory=$false)]
    [switch] $buildTest = $false
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

function Get-VSCMakeGen {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

    $version = &$vswhere -latest -property catalog_productLineVersion
    $year = &$vswhere -latest -property catalog_featureReleaseYear

    return "Visual Studio $version $year"
}

function Get-VSExecPath {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    return &$vswhere -latest -property productPath
}

Write-Host ">>>>> Generating Horseradish project ($($($buildTool -eq "ninja") ? "Ninja" : "VStudio"))"
Write-Host ">>>>> (good luck)"
Write-Host ""

# prepare stuff related to VCPKG

$env:PATH = "$env:VCPKG_DISABLE_METRICS;$env:PATH"

if (-not (Test-Path -Path "build-tools/vcpkg")) {
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

$vcpkgPath = Join-Path -Path "$(Get-Location)" -ChildPath "build-tools/vcpkg"
$env:VCPKG_ROOT = $vcpkgPath
$env:PATH = "$env:VCPKG_ROOT;$env:PATH"

# prepare stuff related to the build target folder and utils (i.e.: download ninja if necessary)

if ($buildTool -eq "ninja") {
    $targetFolder = "build-ninja"

    if (-not (Test-Path -Path "build-tools/ninja")) {
        Write-Host "Downloading Ninja..."

        New-Item -Path "." -Name "build-tools/ninja" -ItemType Directory | Out-Null

        Push-Location -Path "build-tools/ninja"
        try {
            Start-BitsTransfer -Source "https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip"
            Expand-Archive -Path "ninja-win.zip" -DestinationPath "./"
            Remove-Item -Path "./ninja-win.zip"
        } finally {
            Pop-Location
        }
    }

    $ninjaPath = Resolve-Path -Path "./build-tools/ninja"
    if (-not (Test-Path -Path $ninjaPath)) {
        Write-Error "Unable to figure out the path for ninja-build"
    }

    $env:PATH += ";$ninjaPath"
    Set-VSEnv

    Write-Host ""

} else {
    $targetFolder = "build-vs"
}

if (-not (Test-Path -Path $targetFolder)) {
    New-Item -Path "." -Name $targetFolder -ItemType Directory | Out-Null
}

# now we can run cmake to generate the build solution / files

Push-Location -Path $targetFolder
try {

    $env:CMAKE_TOOLCHAIN_FILE = "$(Join-Path -Path "$vcpkgPath" -ChildPath "scripts/buildsystems/vcpkg.cmake")"

    if ($buildTool -eq "ninja") {

        & cmake -DHR_ENABLE_DEVEL=1 -DHR_ENABLE_LOGGING=1 -DHR_ENABLE_PROFILLING=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G "Ninja" ..
        if ($LASTEXITCODE -ne 0) { return; }

        if ($buildTest) {
            cmake --build .
            ctest -V
        }

    } else {
        $generator = Get-VSCMakeGen
        Write-Host "Using generator: $generator"

        & cmake -DHR_ENABLE_DEVEL=1 -DHR_ENABLE_LOGGING=1 -DHR_ENABLE_PROFILLING=1 --fresh -G "$generator" -A x64 -T host=x64 ..
        if ($LASTEXITCODE -ne 0) { return; }

        if ($buildTest) {

            cmake --build . --config Release
            ctest --build-config Release --verbose

        } else {

            $idePath = Get-VSExecPath
            &$idePath .\mustard.slnx

        }

    }

} finally {
    Pop-Location
}

Write-Host ">>>>> done"
