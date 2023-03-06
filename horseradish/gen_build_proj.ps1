Write-Host ">>>>> Generating Horseradish project"
Write-Host ">>>>> (good luck)"

$targetFolder = "build"

if (-not (Test-Path -Path $targetFolder)) {
    New-Item -Path "." -Name $targetFolder -ItemType Directory | Out-Null
}

Push-Location -Path $targetFolder

$env:VULKAN_SDK="C:\dev\VulkanSDK\1.3.239.0\"

& cmake -DHR_ENABLE_DEVEL=1 -DHR_ENABLE_LOGGING=1 -DHR_ENABLE_PROFILLING=1 -G "Visual Studio 17 2022" -A x64 -T host=x64 ..
if ($LASTEXITCODE -ne 0) { Pop-Location; return; }

Pop-Location

Invoke-Item -Path ".\$targetFolder\Mustard.sln"

Write-Host ">>>>> done"
