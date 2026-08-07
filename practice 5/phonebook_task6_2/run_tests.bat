@echo off
chcp 1251 >nul
call build.bat
if errorlevel 1 exit /b 1
tests.exe
exit /b %errorlevel%
