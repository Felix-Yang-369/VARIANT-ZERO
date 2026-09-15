$ErrorActionPreference='Stop'
$gameRoot=Split-Path -Parent $PSScriptRoot
$gameUrl='http://127.0.0.1:5173'
$gameVite=Join-Path $gameRoot 'node_modules\vite\bin\vite.js'
if(-not (Test-Path -LiteralPath $gameVite)){throw 'Run npm install in the game folder first.'}
$gameReady=$false
try {$gamePage=Invoke-WebRequest -Uri $gameUrl -TimeoutSec 2; $gameReady=$gamePage.Content.Contains('零号变种')} catch {}
if(-not $gameReady){
    $gameLogs=Join-Path $gameRoot '.verification\web-launch'
    New-Item -ItemType Directory -Path $gameLogs -Force | Out-Null
    $gameStamp=Get-Date -Format 'yyyyMMdd-HHmmss'
    $gameArgs=@(('"'+$gameVite+'"'),'--host','127.0.0.1','--port','5173','--strictPort')
    $gameProcess=Start-Process -FilePath (Get-Command node.exe).Source -ArgumentList $gameArgs -WorkingDirectory $gameRoot -WindowStyle Hidden -PassThru -RedirectStandardOutput "$gameLogs\$gameStamp.out.log" -RedirectStandardError "$gameLogs\$gameStamp.err.log"
    for($gameAttempt=0;$gameAttempt -lt 40;$gameAttempt++){
        if($gameProcess.HasExited){throw "Classic game server failed; see $gameLogs"}
        try {$gamePage=Invoke-WebRequest -Uri $gameUrl -TimeoutSec 1; $gameReady=$gamePage.Content.Contains('零号变种')}catch{}
        if($gameReady){break};Start-Sleep -Milliseconds 250
    }
    if(-not $gameReady){Stop-Process -Id $gameProcess.Id;throw 'Classic game server did not become ready.'}
}
Start-Process $gameUrl
