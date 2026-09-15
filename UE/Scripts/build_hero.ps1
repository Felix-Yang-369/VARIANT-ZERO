$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$uat = 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat'
& $uat BuildCookRun "-project=D:\Projects\VariantZeroUE\VariantZeroUE.uproject" -noP4 -platform=Win64 -clientconfig=Development -build '-ubtargs=-NoHotReloadFromIDE' -cook '-map=/Game/VariantZero/Maps/P0_Conservatory_Foundation2' -stage -pak -archive "-archivedirectory=$projectRoot\BuildOutput-P0-Hero" -unattended -utf8output
if ($LASTEXITCODE -ne 0) { throw "Hero build failed: $LASTEXITCODE" }

