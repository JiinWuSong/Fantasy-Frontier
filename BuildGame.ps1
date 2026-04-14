param(
    [switch]$RenderFrontend = $true,
    [switch]$BuildEditor,
    [switch]$BuildGame
)

$projectRoot = Resolve-Path $PSScriptRoot
$projectFile = Join-Path $projectRoot 'FantasyFrontier.uproject'
$engineRoot = 'C:\Program Files\Epic Games\UE_5.7'
$buildScript = Join-Path $engineRoot 'Engine\Build\BatchFiles\Build.bat'
$frontendScript = Join-Path $projectRoot 'Tools\Branding\render_intro_assets.ps1'

if (-not (Test-Path $projectFile)) {
    throw "Project file not found: $projectFile"
}

if (-not (Test-Path $buildScript)) {
    throw "UE5 build script not found: $buildScript"
}

if ($RenderFrontend) {
    powershell -ExecutionPolicy Bypass -File $frontendScript
    if ($LASTEXITCODE -ne 0) {
        throw 'Frontend asset render failed.'
    }
}

if ($BuildEditor) {
    & $buildScript FantasyFrontierEditor Win64 Development "-Project=$projectFile" -WaitMutex -NoHotReloadFromIDE
    if ($LASTEXITCODE -ne 0) {
        throw 'Editor build failed.'
    }
}

if ($BuildGame) {
    & $buildScript FantasyFrontier Win64 Development "-Project=$projectFile" -WaitMutex -NoHotReloadFromIDE
    if ($LASTEXITCODE -ne 0) {
        throw 'Game build failed.'
    }
}
