@echo off
setlocal

where gcc >nul 2>nul
if errorlevel 1 (
    echo ERROR: gcc was not found in PATH.
    exit /b 1
)

where ar >nul 2>nul
if errorlevel 1 (
    echo ERROR: ar was not found in PATH.
    exit /b 1
)

set CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251

echo [1/7] Compiling contact_book.c...
gcc %CFLAGS% -c contact_book.c -o contact_book.o
if errorlevel 1 exit /b 1

echo [2/7] Creating static library libcontactbook.a...
ar rcs libcontactbook.a contact_book.o
if errorlevel 1 exit /b 1

echo [3/7] Compiling application modules...
gcc %CFLAGS% -c main.c -o main.o
if errorlevel 1 exit /b 1
gcc %CFLAGS% -c input.c -o input.o
if errorlevel 1 exit /b 1
gcc %CFLAGS% -c storage.c -o storage.o
if errorlevel 1 exit /b 1

echo [4/7] Linking phonebook_static.exe with libcontactbook.a...
gcc %CFLAGS% main.o input.o storage.o -L. -lcontactbook -o phonebook_static.exe
if errorlevel 1 exit /b 1

echo [5/7] Compiling tests...
gcc %CFLAGS% -c tests.c -o tests.o
if errorlevel 1 exit /b 1

echo [6/7] Linking tests.exe with libcontactbook.a...
gcc %CFLAGS% tests.o storage.o -L. -lcontactbook -o tests.exe
if errorlevel 1 exit /b 1

echo [7/7] Done.
echo Static library contents:
ar t libcontactbook.a

echo.
echo Build completed successfully.
echo Run tests: tests.exe
echo Run program: phonebook_static.exe
endlocal
