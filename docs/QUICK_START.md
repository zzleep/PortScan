# 🚀 Quick Start Guide - PortScan

## Windows

### GUI Version (Recommended)

```powershell
cd gui_imgui
.\build_imgui.bat    # Downloads ImGui automatically
.\run_gui.bat        # Launch the application
```

### CLI Version

```powershell
cd cli
.\build.bat
.\port_scanner.exe 192.168.1.1 -p 1-1000
```

## Linux

### CLI Version

```bash
cd cli
chmod +x build.sh
./build.sh
./port_scanner 192.168.1.1 -p 1-1000
```

## Usage Examples

### GUI
1. Enter target IP or hostname
2. Select a preset or enter custom ports
3. Click "Start Scan"
4. View color-coded results
5. Export if needed

### CLI
```bash
# Quick scan
./port_scanner 192.168.1.1

# Specific ports
./port_scanner example.com -p 80,443,8080

# Port range with more threads
./port_scanner 10.0.0.1 -p 1-10000 -t 200

# Common ports only
./port_scanner scanme.nmap.org --common
```

## Presets (GUI)

- **Quick Scan** - Common ports only
- **Standard** - Ports 1-1024
- **Full Scan** - All 65535 ports
- **Web Services** - HTTP/HTTPS ports
- **Databases** - MySQL, PostgreSQL, MongoDB, etc.

## Tips

- Start with 100 threads (default)
- Increase to 200-300 for faster scans
- Use timeout 2-3 for slow networks
- Export results before clearing

## Legal Notice

⚠️ **Only scan authorized systems!**

Get written permission before scanning any network or system you don't own.

---

## What's New in v1.0.1 ✨

### Fixed Bugs ✅
- **Help > About** menu now works properly
- **Stop button** responds instantly (was 5-10 seconds)
- **Clear button** disabled during scan (prevents visual glitches)
- **No console window** appears when launching GUI

### Performance Improvements ⚡
- **30% faster** scanning with non-blocking sockets
- Better thread management and cleanup
- Instant stop response with optimized cancellation

### UI Improvements 🎨
- Added "Stopping..." visual feedback
- Enhanced About dialog with feature list
- Better button state management

---

For more details, see the main [README.md](../README.md)
