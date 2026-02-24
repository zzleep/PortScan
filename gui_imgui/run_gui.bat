@echo off
echo ========================================
echo   Starting Port Scanner GUI
echo ========================================
echo.

if not exist "PortScannerGUI.exe" (
    echo [ERROR] PortScannerGUI.exe not found!
    echo.
    echo Please build it first by running: build_imgui.bat
    echo.
    pause
    exit /b 1
)

echo Starting application...
echo.
start "" "PortScannerGUI.exe"

echo Application launched!
echo You can close this window.
timeout /t 2 /nobreak >nul
