@echo off
setlocal
set /p vzPackage=<"%~dp0UE\当前试玩包.txt"
start "" "%~dp0UE\%vzPackage%\Windows\VariantZeroUE.exe" -VZStory -windowed -ResX=1920 -ResY=1080
