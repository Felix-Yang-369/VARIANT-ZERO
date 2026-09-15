$ErrorActionPreference='Stop'
$taskRoot=Split-Path -Parent $PSScriptRoot
$taskExe=Join-Path $taskRoot 'BuildOutput-Growth\Windows\VariantZeroUE\Binaries\Win64\VariantZeroUE.exe'
$taskResults=@()
foreach($taskTest in @(@('VZChuyaStoryTest','VZ_CHUYA_STORY: PASS'),@('VZTravelTest','VZ_TRAVEL: PASS'),@('VZGardenTest','VZ_GARDEN: PASS'),@('VZStoryTest','VZ_STORY: PASS'),@('VZHeroTest','VZ_HERO: PASS'),@('VZSmokeTest','VZ_SMOKE: PASS'),@('VZCombatTest','VZ_COMBAT: PASS'),@('VZOrderTest','VZ_ORDERS: PASS'),@('VZMenuTest','VZ_MENU: PASS'))){
    $taskLog=Join-Path $taskRoot ('growth-'+$taskTest[0]+'.log')
    $taskArgs="-$($taskTest[0]) -windowed -ForceRes -ResX=1920 -ResY=1080 -unattended -abslog=$taskLog"
    if($taskTest[0] -ne 'VZMenuTest'){$taskArgs+=' -nosound'}
    $taskProcess=Start-Process -FilePath $taskExe -ArgumentList $taskArgs -WindowStyle Hidden -PassThru
    if(-not $taskProcess.WaitForExit(60000)){Stop-Process -Id $taskProcess.Id;throw "$($taskTest[0]) timeout"}
    $taskProcess.Refresh()
    $taskText=[IO.File]::ReadAllText($taskLog)
    $taskPass=$taskProcess.ExitCode -eq 0 -and $taskText.Contains($taskTest[1]) -and $taskText -notmatch 'VZ_\w+: FAIL'
    $taskResults+=@{test=$taskTest[0];passed=$taskPass;exitCode=$taskProcess.ExitCode;log=$taskLog}
    $taskResults | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $taskRoot 'Docs\growth-verification.json') -Encoding utf8
    "$($taskTest[0]): $taskPass exit=$($taskProcess.ExitCode)"
    if(-not $taskPass){throw "$($taskTest[0]) failed; see $taskLog"}
}













