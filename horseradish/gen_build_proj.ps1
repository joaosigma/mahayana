param (
    [parameter(Mandatory=$false)]
    [ValidateSet('x86', 'x64', IgnoreCase = $true)]
    [string]$targetArch = 'x64'
)

Write-Host ">>>>> Generating Horseradish project ($targetArch)"
Write-Host ">>>>> (good luck)"

$targetFolder = if ($targetArch -eq "x86") {"build-cmake-x86"} else {"build-cmake-x64"}

if (-not (Test-Path -Path $targetFolder)) {
    New-Item -Path "." -Name $targetFolder -ItemType Directory | Out-Null
}

Push-Location -Path $targetFolder

if ($targetArch -eq "x86") {
    & cmake -DHR_ENABLE_DEVEL=1 -DHR_ENABLE_LOGGING=1 -DHR_ENABLE_PROFILLING=1 -G "Visual Studio 17 2022" -A Win32 -T host=x64 ..
    if ($LASTEXITCODE -ne 0) { Pop-Location; return; }
}
else {
    & cmake -DHR_ENABLE_DEVEL=1 -DHR_ENABLE_LOGGING=1 -DHR_ENABLE_PROFILLING=1 -G "Visual Studio 17 2022" -A x64 -T host=x64 ..
    if ($LASTEXITCODE -ne 0) { Pop-Location; return; }
}


Pop-Location

Invoke-Item -Path ".\$targetFolder\Mustard.sln"

Write-Host ">>>>> done"
