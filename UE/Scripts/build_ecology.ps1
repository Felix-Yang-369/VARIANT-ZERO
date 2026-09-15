$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$uat = 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat'
& $uat BuildCookRun "-project=D:\Projects\VariantZeroUE\VariantZeroUE.uproject" -noP4 -platform=Win64 -clientconfig=Development -build '-ubtargs=-NoHotReloadFromIDE' -cook '-map=/Game/VariantZero/Maps/P0_Conservatory_Ecology' -stage -pak -archive "-archivedirectory=$projectRoot\BuildOutput-P0-Ecology" -unattended -utf8output
if ($LASTEXITCODE -ne 0) { throw "Ecology build failed: $LASTEXITCODE" }

