param(
    [int]$Width = 1920,
    [int]$Height = 1080,
    [int]$DurationSeconds = 12
)

$projectRoot = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$backgroundPath = Join-Path $projectRoot 'Content\Slate\MenuBackground.png'
$moviePath = Join-Path $projectRoot 'Content\Movies\FantasyFrontierIntro.mp4'
$pythonScript = Join-Path $projectRoot 'Tools\Branding\generate_frontend_art.py'
$fontPath = Join-Path $projectRoot 'Content\Slate\Fonts\Georgia-Bold.ttf'

$pythonCommand = Get-Command python -ErrorAction SilentlyContinue
if (-not $pythonCommand) {
    throw 'Python was not found on PATH.'
}

$ffmpegCommand = Get-Command ffmpeg -ErrorAction SilentlyContinue
if (-not $ffmpegCommand) {
    $fallbackFfmpeg = Join-Path $projectRoot 'Tools\bin\ffmpeg.cmd'
    if (Test-Path $fallbackFfmpeg) {
        $ffmpegCommand = Get-Item $fallbackFfmpeg
    }
}
if (-not $ffmpegCommand) {
    throw 'ffmpeg was not found on PATH.'
}

if (-not (Test-Path $fontPath)) {
    throw "Expected font at $fontPath"
}

& $pythonCommand.Source $pythonScript --output $backgroundPath --width $Width --height $Height
if ($LASTEXITCODE -ne 0) {
    throw 'Background generation failed.'
}

$fontForFfmpeg = $fontPath.Replace('\', '/').Replace(':', '\:')
$titleFade = "if(lt(t\,6.9)\,0\,if(lt(t\,8.7)\,(t-6.9)/1.8\,if(lt(t\,10.9)\,1\,max(0\,(12.0-t)/1.1))))"
$filterParts = @(
    "zoompan=z='min(1.11,1+0.00033*on)':x='iw/2-(iw/zoom/2)+sin(on/52)*10':y='ih/2-(ih/zoom/2)-18+sin(on/80)*6':d=1:s=${Width}x${Height}:fps=30",
    "eq=saturation=1.10:brightness=0.02",
    "vignette=PI/5",
    "drawtext=fontfile='${fontForFfmpeg}':text='FANTASY FRONTIER':fontsize=108:fontcolor=0xF7E8C7:borderw=2:bordercolor=0x2E1C08@0.85:shadowx=0:shadowy=4:shadowcolor=0xA36F2A@0.45:x=(w-text_w)/2:y=h*0.44:alpha='${titleFade}'",
    "drawbox=x=(w-1540)/2:y=h*0.56:w=1540:h=3:color=0xC89A40@0.92:t=fill:enable='between(t,7.2,11.6)'",
    "fade=t=in:st=0:d=0.8",
    "fade=t=out:st=11.1:d=0.9"
)
$filter = $filterParts -join ','
$ffmpegArgs = @(
    '-y',
    '-loop', '1',
    '-framerate', '30',
    '-i', $backgroundPath,
    '-t', $DurationSeconds,
    '-vf', $filter,
    '-c:v', 'libx264',
    '-pix_fmt', 'yuv420p',
    '-movflags', '+faststart',
    $moviePath
)

& $ffmpegCommand.Source @ffmpegArgs
if ($LASTEXITCODE -ne 0) {
    throw 'Intro movie rendering failed.'
}

Write-Output "Background: $backgroundPath"
Write-Output "Movie: $moviePath"
