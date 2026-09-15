@echo off
chcp 65001 >nul
if exist "%LOCALAPPDATA%\VariantZero\SaveGames\Story" (
    start "" explorer.exe "%LOCALAPPDATA%\VariantZero\SaveGames\Story"
) else (
    echo 首次保存后，存档会出现在这里：
    echo %LOCALAPPDATA%\VariantZero\SaveGames\Story
    pause
)
