@echo off
REM Build script for Windows using MinGW

echo Building C++ Port Scanner...

REM Check if g++ is available
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo Error: g++ not found. Please install MinGW-w64.
    echo Download from: https://www.mingw-w64.org/
    exit /b 1
)

REM Compile the project
echo Compiling...
g++ -std=c++17 -Wall -Wextra -O2 -pthread -o port_scanner.exe main.cpp PortScanner.cpp -lws2_32

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful! Executable: port_scanner.exe
    echo.
    echo Usage: port_scanner.exe ^<target^> [options]
    echo Example: port_scanner.exe 192.168.1.1 -p 1-1000
    echo.
    echo Run 'port_scanner.exe --help' for more information.
) else (
    echo.
    echo Build failed! Please check the error messages above.
    exit /b 1
)
