@echo off
setlocal enabledelayedexpansion

tasklist | findstr /I "bridgecommand-es" > nul
if %errorlevel% equ 0 (
    echo ------ Stop EnetServer

    for /f "tokens=2 delims= " %%A in ('tasklist ^| findstr /I "bridgecommand-es"') do (
        set PID=%%A
        taskkill /F /PID !PID!
    )
) else (
    echo ------- EnetServer is already stopped
)

endlocal
