@echo off
setlocal enabledelayedexpansion

echo ============================================================
echo   Building Library Management System (C Program)
echo ============================================================

REM Check if gcc is available in current PATH
where gcc >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    if exist "%LOCALAPPDATA%\Programs\w64devkit\bin\gcc.exe" (
        set "PATH=%LOCALAPPDATA%\Programs\w64devkit\bin;%PATH%"
        echo [+] Added GCC to temporary PATH
    ) else (
        echo [!] GCC not found. Please install MinGW or GCC.
        pause
        exit /b 1
    )
)

echo [+] Compiling library.c ...
gcc -Wall -Wextra -O2 library.c -o library.exe

if %ERRORLEVEL% EQU 0 (
    echo ============================================================
    echo [SUCCESS] Compiled 'library.exe' successfully!
    echo ============================================================
    echo 1. To run the C console program, run: library.exe
    echo 2. To open the Web UI, simply double-click 'index.html' (No server required!)
    exit /b 0
)

echo [!] Compilation failed with error code %ERRORLEVEL%
pause
exit /b %ERRORLEVEL%
