$ErrorActionPreference = 'Stop'
$assetDir = [IO.Path]::GetFullPath($PSScriptRoot)
$fileNames = [ordered]@{
    'variant-zero-sanctuary-v1.blend' = '原初生态研究所_V-001_原豆母体_v1.blend'
    'sanctuary-preview.png' = '原初生态研究所_全景_v1.png'
    'legume-podium-preview.png' = 'V-001_原豆母体_培养台近景_v1.png'
    'sanctuary-preview.png0001.png' = '原初生态研究所_全景_v1_导出01.png'
    'sanctuary-preview.png0002.png' = '原初生态研究所_全景_v1_导出02.png'
}
foreach ($entry in $fileNames.GetEnumerator()) {
    $sourcePath = Join-Path $assetDir $entry.Key
    $targetPath = Join-Path $assetDir $entry.Value
    if ((Test-Path -LiteralPath $sourcePath) -and (Test-Path -LiteralPath $targetPath)) { throw "Target already exists: $targetPath" }
}
$replacements = [ordered]@{
    'variant-zero-sanctuary-v1.blend' = '原初生态研究所_V-001_原豆母体_v1.blend'
    'sanctuary-preview.png' = '原初生态研究所_全景_v1.png'
    'legume-podium-preview.png' = 'V-001_原豆母体_培养台近景_v1.png'
    '01 • Conservatory | 温室建筑' = '01 • 原初生态研究所 | 温室建筑'
    '05 • V-001 原豆母体' = '05 • V-001 原豆母体 | 豆科'
    'CAM 01 • Complete sanctuary' = 'CAM 01 • 原初生态研究所全景'
    'CAM 02 • Seed and podium' = 'CAM 02 • 原豆母体与培养台'
    'CAM 03 • Character front' = 'CAM 03 • 原豆母体正面'
    'V-001 •' = 'V-001 原豆母体 •'
}
$editableFiles = @('build_sanctuary.py','refine_scene.py','refine_saved.py','finalize_scene.py','verify_scene.py','README.md','scene-manifest.json','verification.json')
foreach ($name in $editableFiles) {
    $filePath = Join-Path $assetDir $name
    $contents = [IO.File]::ReadAllText($filePath)
    foreach ($entry in $replacements.GetEnumerator()) { $contents = $contents.Replace($entry.Key, $entry.Value) }
    [IO.File]::WriteAllText($filePath, $contents, [Text.UTF8Encoding]::new($false))
}
foreach ($entry in $fileNames.GetEnumerator()) {
    $sourcePath = Join-Path $assetDir $entry.Key
    if (Test-Path -LiteralPath $sourcePath) {
        Rename-Item -LiteralPath $sourcePath -NewName $entry.Value
        Write-Output "Renamed: $($entry.Key) -> $($entry.Value)"
    }
}
