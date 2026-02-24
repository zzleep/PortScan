#!/bin/bash
# Build script for Linux/Unix

echo "Building C++ Port Scanner..."

# Check if g++ is available
if ! command -v g++ &> /dev/null; then
    echo "Error: g++ not found. Please install it:"
    echo "  Ubuntu/Debian: sudo apt-get install build-essential"
    echo "  Fedora/RHEL: sudo dnf install gcc-c++"
    echo "  macOS: xcode-select --install"
    exit 1
fi

# Compile the project
echo "Compiling..."
g++ -std=c++17 -Wall -Wextra -O2 -pthread -o port_scanner main.cpp PortScanner.cpp

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful! Executable: ./port_scanner"
    echo ""
    echo "Usage: ./port_scanner <target> [options]"
    echo "Example: ./port_scanner 192.168.1.1 -p 1-1000"
    echo ""
    echo "Run './port_scanner --help' for more information."
    
    # Make executable
    chmod +x port_scanner
else
    echo ""
    echo "Build failed! Please check the error messages above."
    exit 1
fi
