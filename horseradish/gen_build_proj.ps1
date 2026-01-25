param (
    # target Ninja instead of VStudio
    [Parameter(Mandatory=$false)]
    [switch ]$Ninja
)

$ErrorActionPreference = 'Stop'

function Set-VSEnv {
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

Write-Host ">>>>> Generating Horseradish project ($($Ninja ? "Ninja" : "VStudio"))"
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

if ($Ninja) {
    $targetFolder = "build-ninja"

    if (-not (Test-Path -Path "build-tools")) {
        New-Item -Path "." -Name "build-tools" -ItemType Directory | Out-Null
    }
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

    $Env:PATH += ";$ninjaPath"

    Set-VSEnv

} else {
    $targetFolder = "build-vs"
}

if (-not (Test-Path -Path $targetFolder)) {
    New-Item -Path "." -Name $targetFolder -ItemType Directory | Out-Null
}

Push-Location -Path $targetFolder
try {

    if ($Ninja) {
        & cmake -DHR_ENABLE_DEVEL=1 -DHR_ENABLE_LOGGING=1 -DHR_ENABLE_PROFILLING=1 -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -G "Ninja" ..
        if ($LASTEXITCODE -ne 0) { return; }
    } else {
        $generator = Get-VSCMakeGen
        Write-Host "Using generator: $generator"

        & cmake -DHR_ENABLE_DEVEL=1 -DHR_ENABLE_LOGGING=1 -DHR_ENABLE_PROFILLING=1 -DCMAKE_TOOLCHAIN_FILE="$(Join-Path -Path "$vcpkgPath" -ChildPath "scripts/buildsystems/vcpkg.cmake")" -G "$generator" -A x64 -T host=x64 ..
        if ($LASTEXITCODE -ne 0) { return; }
    }

} finally {
    Pop-Location
}

# if (-not $Ninja) { Invoke-Item -Path ".\$targetFolder\Mustard.sln" }

Write-Host ">>>>> done"
