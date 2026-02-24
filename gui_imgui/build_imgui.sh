#!/bin/bash
# Build script for Linux

echo "========================================"
echo "  Building Port Scanner - Linux"
echo "========================================"
echo ""

# Check if g++ is available
if ! command -v g++ &> /dev/null; then
    echo "[ERROR] g++ not found!"
    echo ""
    echo "Please install build essentials:"
    echo "  Ubuntu/Debian: sudo apt-get install build-essential"
    echo "  Fedora/RHEL: sudo dnf install gcc-c++"
    echo ""
    exit 1
fi

echo "[1/3] Downloading Dear ImGui..."
# Download ImGui if not present
if [ ! -d "../imgui" ]; then
    wget https://github.com/ocornut/imgui/archive/refs/tags/v1.90.4.tar.gz
    tar -xzf v1.90.4.tar.gz
    mv imgui-1.90.4 ../imgui
    rm v1.90.4.tar.gz
else
    echo "ImGui already downloaded"
fi

echo ""
echo "[2/3] Compiling..."

# Compile ImGui sources
g++ -std=c++17 -O2 -c ../imgui/imgui.cpp -I../imgui -o imgui.o
g++ -std=c++17 -O2 -c ../imgui/imgui_draw.cpp -I../imgui -o imgui_draw.o
g++ -std=c++17 -O2 -c ../imgui/imgui_tables.cpp -I../imgui -o imgui_tables.o
g++ -std=c++17 -O2 -c ../imgui/imgui_widgets.cpp -I../imgui -o imgui_widgets.o

# Note: Linux version would use OpenGL backend instead of DirectX
# This is a simplified version - full Linux GUI support would need OpenGL/SDL

echo "Note: GUI version on Linux requires additional setup for OpenGL/SDL backends"
echo "CLI version is recommended for Linux"

echo ""
echo "Building CLI version instead..."
cd ../cli
./build.sh

echo ""
echo "========================================"
echo "  Build Complete!"
echo "========================================"
echo "  Use the CLI version: ./port_scanner"
echo "========================================"
