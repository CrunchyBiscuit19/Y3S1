@echo off

if "%~2"=="" (
    echo Usage: download.bat REMOTE_PATH LOCAL_PATH
    echo Example: download.bat ~/a.txt a.txt
    exit /b 1
)

set REMOTE_PATH=%~1
set LOCAL_PATH=%~2

rem Strip trailing slash/backslash so the quoted path is not mangled by scp
if "%LOCAL_PATH:~-1%"=="\" set LOCAL_PATH=%LOCAL_PATH:~0,-1%
if "%LOCAL_PATH:~-1%"=="/" set LOCAL_PATH=%LOCAL_PATH:~0,-1%

scp -r -J simyy@stujump.comp.nus.edu.sg e1398738@soctf-pdc-001.comp.nus.edu.sg:%REMOTE_PATH% "%LOCAL_PATH%"
