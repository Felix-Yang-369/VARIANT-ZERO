$ErrorActionPreference='Stop'
$taskRoot=Join-Path (Split-Path -Parent $PSScriptRoot) 'ArtSource\PolyHaven'
New-Item -ItemType Directory -Force -Path $taskRoot | Out-Null
$taskManifest=@()
foreach($taskId in @('fern_02','boulder_01','rock_01','forest_ground_04')){
    $taskData=Invoke-RestMethod "https://api.polyhaven.com/files/$taskId" -Headers @{'User-Agent'='VariantZeroAssetImport/1.0'}
    $taskDir=Join-Path $taskRoot $taskId
    New-Item -ItemType Directory -Force -Path $taskDir | Out-Null
    $taskSelected=@{}
    if($taskData.fbx){$taskSelected['mesh']=$taskData.fbx.'2k'.fbx}
    foreach($taskType in @('Diffuse','nor_dx','Rough','Alpha')){
        if($taskData.$taskType){
            $taskFormats=$taskData.$taskType.'2k'
            $taskSelected[$taskType]=if($taskFormats.png){$taskFormats.png}else{$taskFormats.jpg}
        }
    }
    foreach($taskPair in $taskSelected.GetEnumerator()){
        $taskFile=$taskPair.Value
        if(-not $taskFile.url){throw "Missing file metadata $taskId $($taskPair.Key)"}
        $taskPath=Join-Path $taskDir ([IO.Path]::GetFileName(([Uri]$taskFile.url).AbsolutePath))
        if(-not(Test-Path -LiteralPath $taskPath)){Invoke-WebRequest $taskFile.url -OutFile $taskPath}
        $taskHash=(Get-FileHash -LiteralPath $taskPath -Algorithm MD5).Hash.ToLowerInvariant()
        if($taskHash -ne $taskFile.md5){throw "Checksum mismatch $taskPath"}
        $taskManifest+=@{asset=$taskId;kind=$taskPair.Key;file=$taskPath;url=$taskFile.url;md5=$taskHash;license='CC0';source="https://polyhaven.com/a/$taskId"}
    }
}
$taskManifest | ConvertTo-Json -Depth 5 | Set-Content (Join-Path $taskRoot 'manifest.json') -Encoding utf8
'Verified downloads: '+$taskManifest.Count
