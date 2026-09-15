$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe' "$projectRoot\VariantZeroUE.uproject" -NullRHI -Unattended -NoSound -UTF8Output '-ExecCmds=Automation RunTests VariantZero' '-TestExit=Automation Test Queue Empty' "-ReportExportPath=$projectRoot\TestResults\Native" "-abslog=$projectRoot\native-tests.log"
if ($LASTEXITCODE -ne 0) { throw "Native tests failed: $LASTEXITCODE" }
$report = Get-Content -LiteralPath "$projectRoot\TestResults\Native\index.json" -Raw | ConvertFrom-Json
if ($report.failed -ne 0 -or $report.succeeded -ne 11 -or $report.notRun -ne 0) { throw 'Expected all eleven VariantZero suites to pass.' }





