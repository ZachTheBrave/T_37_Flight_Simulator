@echo off
set "gamePath=C:\X-Plane 12\X-Plane.exe"
set "programName=X-Plane"

goto check

:launch
echo Waiting 3 seconds until X-Plane launches...
timeout /t 3 /nobreak
echo X-Plane is launching...
start "" "%gamePath%"

timeout /t 60 /nobreak >nul

:check
powershell -Command "if (Get-Process | Where-Object { $_.Name -eq '%programName%' }) { exit 0 } else { exit 1 }"
if "%ERRORLEVEL%"=="0" (
    rem X-Plane is running.
) else (
    echo X-Plane is not running. Launching...
    goto launch
)
timeout /t 15 /nobreak >nul
goto check

