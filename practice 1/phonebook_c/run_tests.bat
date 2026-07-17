@echo off
setlocal
cd /d "%~dp0"
chcp 65001 >nul

if not exist tests.exe (
    echo ERROR: tests.exe was not found. Run build.bat first.
    exit /b 1
)

tests.exe
exit /b %ERRORLEVEL%
