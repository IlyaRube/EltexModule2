@echo off
setlocal

where gcc >nul 2>nul
if errorlevel 1 (
    echo ERROR: gcc was not found in PATH.
    exit /b 1
)

set CFLAGS=-std=c11 -Wall -Wextra -Wpedantic -finput-charset=UTF-8 -fexec-charset=CP1251

echo [1/7] Compiling contact_book.c for DLL...
gcc %CFLAGS% -DCONTACT_BOOK_BUILD_DLL -c contact_book.c -o contact_book_dll.o
if errorlevel 1 exit /b 1

echo [2/7] Creating dynamic library contactbook.dll...
gcc -shared contact_book_dll.o -Wl,--out-implib,libcontactbook.dll.a -o contactbook.dll
if errorlevel 1 exit /b 1

echo [3/7] Compiling application modules...
gcc %CFLAGS% -c main.c -o main.o
if errorlevel 1 exit /b 1
gcc %CFLAGS% -c input.c -o input.o
if errorlevel 1 exit /b 1
gcc %CFLAGS% -c storage.c -o storage.o
if errorlevel 1 exit /b 1

echo [4/7] Linking phonebook_dynamic.exe with contactbook.dll...
gcc %CFLAGS% main.o input.o storage.o -L. -lcontactbook -o phonebook_dynamic.exe
if errorlevel 1 exit /b 1

echo [5/7] Compiling tests...
gcc %CFLAGS% -c tests.c -o tests.o
if errorlevel 1 exit /b 1

echo [6/7] Linking tests.exe with contactbook.dll...
gcc %CFLAGS% tests.o storage.o -L. -lcontactbook -o tests.exe
if errorlevel 1 exit /b 1

echo [7/7] Build completed successfully.
echo.
echo Dynamic library: contactbook.dll
echo Import library:  libcontactbook.dll.a
echo Run tests: tests.exe
echo Run program: phonebook_dynamic.exe
endlocal
