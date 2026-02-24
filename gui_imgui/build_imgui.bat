@echo off
echo ========================================
echo   Port Scanner - Dear ImGui Build
echo ========================================
echo.

REM Check if Dear ImGui exists
if not exist "imgui" (
    echo [1/3] Dear ImGui not found. Downloading...
    powershell -ExecutionPolicy Bypass -File download_imgui.ps1
    if %ERRORLEVEL% NEQ 0 (
        echo [ERROR] Failed to download Dear ImGui
        pause
        exit /b 1
    )
) else (
    echo [INFO] Dear ImGui found
)

echo.
echo [2/3] Compiling...

REM Compile ImGui sources
echo Compiling imgui.cpp...
g++ -std=c++17 -O2 -c imgui/imgui.cpp -Iimgui -o imgui.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

echo Compiling imgui_demo.cpp...
g++ -std=c++17 -O2 -c imgui/imgui_demo.cpp -Iimgui -o imgui_demo.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

echo Compiling imgui_draw.cpp...
g++ -std=c++17 -O2 -c imgui/imgui_draw.cpp -Iimgui -o imgui_draw.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

echo Compiling imgui_tables.cpp...
g++ -std=c++17 -O2 -c imgui/imgui_tables.cpp -Iimgui -o imgui_tables.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

echo Compiling imgui_widgets.cpp...
g++ -std=c++17 -O2 -c imgui/imgui_widgets.cpp -Iimgui -o imgui_widgets.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

echo Compiling imgui_impl_win32.cpp...
g++ -std=c++17 -O2 -c imgui/backends/imgui_impl_win32.cpp -Iimgui -Iimgui/backends -o imgui_impl_win32.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

echo Compiling imgui_impl_dx11.cpp...
g++ -std=c++17 -O2 -c imgui/backends/imgui_impl_dx11.cpp -Iimgui -Iimgui/backends -o imgui_impl_dx11.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

REM Compile application sources
echo Compiling PortScannerImGui.cpp...
g++ -std=c++17 -O2 -c PortScannerImGui.cpp -Iimgui -o PortScannerImGui.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

echo Compiling main_imgui.cpp...
g++ -std=c++17 -O2 -c main_imgui.cpp -Iimgui -Iimgui/backends -o main_imgui.o
if %ERRORLEVEL% NEQ 0 goto :compile_error

echo All files compiled successfully!

echo.
echo [3/3] Linking...

REM Link everything together (with -mwindows to hide console)
g++ -mwindows -o PortScannerGUI.exe ^
    imgui.o imgui_demo.o imgui_draw.o imgui_tables.o imgui_widgets.o ^
    imgui_impl_win32.o imgui_impl_dx11.o ^
    PortScannerImGui.o main_imgui.o ^
    -ld3d11 -ldxgi -ld3dcompiler -lws2_32 -lgdi32 -ldwmapi -limm32

if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Linking failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo   Build Successful!
echo ========================================
echo   Executable: PortScannerGUI.exe
echo   Run it to start the port scanner
echo ========================================
echo.

REM Clean up object files
del *.o 2>nul

echo Cleaned up temporary files
echo.
pause
goto :eof

:compile_error
echo.
echo [ERROR] Compilation failed!
echo Please check the error messages above.
echo.
echo Common issues:
echo   - Make sure g++ is installed and in PATH
echo   - Make sure Dear ImGui was downloaded correctly
echo   - Check that all source files exist
echo.
pause
exit /b 1
