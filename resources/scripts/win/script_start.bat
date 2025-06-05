@echo off
setlocal

tasklist | findstr /I "bridgecommand-es.exe" > serverStatus.log

for /f %%i in ('find /c /v "" ^< serverStatus.log') do set COUNT=%%i

if %COUNT% == 0 (
	echo -------- Start EnetServer
    cd /d "%USERPROFILE%\bc\bin\win"
    bridgecommand-es.exe
) else (
    echo -------- EnetServer is already running
)

del serverStatus.log
