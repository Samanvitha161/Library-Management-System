@echo off
cd /d "%~dp0"

echo ============================================================
echo         LIBRARY MANAGEMENT SYSTEM (MINI PROJECT)
echo ============================================================
echo.
echo [1] Start Localhost Web App (C backend)
echo [2] Run C Console Program (library.exe)
echo [3] Compile C Program (library.c)
echo [4] Exit
echo.
set /p choice="Enter choice (1-4): "

if "%choice%"=="1" (
    if not exist "library_server.exe" (
        echo [!] library_server.exe was not found.
        echo Building the backend server...
        where gcc >nul 2>nul
        if errorlevel 1 (
            if exist "%LOCALAPPDATA%\Programs\w64devkit\bin\gcc.exe" (
                set "PATH=%LOCALAPPDATA%\Programs\w64devkit\bin;%PATH%"
            ) else (
                echo [!] GCC is needed to build the missing server executable.
                echo You can still open index.html for the standalone frontend.
                pause
                exit /b 1
            )
        )
        gcc -Wall -Wextra -O2 backend/src/main.c backend/src/library.c backend/src/http_server.c backend/src/cJSON_mini.c -Ibackend/include -lws2_32 -o library_server.exe
        if errorlevel 1 (
            echo [!] Backend build failed.
            pause
            exit /b 1
        )
    )

    echo Starting backend server on http://localhost:8080 ...
    start "Library Management Server" /D "%~dp0" "%~dp0library_server.exe"
    timeout /t 2 /nobreak >nul
    start "" "http://localhost:8080"
    exit /b 0
)

if "%choice%"=="2" (
    if not exist "library.exe" (
        echo Compiling library.c first...
        call build.bat
    )
    cls
    library.exe
    exit /b 0
)

if "%choice%"=="3" (
    call build.bat
    pause
    exit /b 0
)

exit /b 0
