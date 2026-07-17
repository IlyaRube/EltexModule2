@echo off
setlocal
cd /d "%~dp0"

where gcc >nul 2>nul
if errorlevel 1 (
    echo ERROR: gcc was not found in PATH.
    exit /b 1
)

gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 main.c input.c contact_book.c storage.c -o phonebook.exe
if errorlevel 1 exit /b 1

gcc -std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251 tests.c contact_book.c storage.c -o tests.exe
if errorlevel 1 exit /b 1

echo Build completed successfully.
endlocal
