$ErrorActionPreference='Stop'
$taskRoot=Split-Path -Parent $PSScriptRoot
& 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat' BuildCookRun "-project=D:\Projects\VariantZeroUE\VariantZeroUE.uproject" -noP4 -platform=Win64 -clientconfig=Development -build '-ubtargs=-NoHotReloadFromIDE' -cook '-map=/Game/VariantZero/Maps/P0_Conservatory_Ecology' -stage -pak -archive "-archivedirectory=$taskRoot\BuildOutput-Pollinator" -unattended -utf8output
if($LASTEXITCODE -ne 0){throw "Pollinator build failed: $LASTEXITCODE"}


