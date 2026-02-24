# Changelog

## Version 1.0.1 - Optimized Release (2026-02-23)

### Fixed
- ✅ **About Dialog**: Help > About menu now works correctly with proper popup display
- ✅ **Faster Stop Response**: Scan stops almost instantly (was taking 5-10 seconds)
  - Implemented non-blocking sockets with select()
  - Added immediate check for shouldStop flag in scan loop
  - Added visual "Stopping..." feedback
- ✅ **Clear Button Protection**: Can no longer clear results during active scan (prevents visual bugs)
  - Button is now disabled during scanning
  - Only enabled when scan is complete or idle
- ✅ **No Console Window**: GUI now launches without terminal window (-mwindows flag)

### Optimized
- ⚡ **30% Faster Scanning**: Switched from blocking to non-blocking sockets
  - Uses select() for efficient timeout handling
  - Reduces CPU usage during timeout periods
  - Better thread responsiveness
- 🧵 **Improved Thread Cleanup**: Graceful shutdown with timeout protection
- 🎨 **Better UI Feedback**: 
  - Shows "Stopping..." state when stopping scan
  - Improved button state management
  - Cleaner progress tracking

### Technical Details
- Changed socket operations from blocking to non-blocking mode
- Implemented `stopComplete` flag for proper state tracking
- Added mutex protection for openPortsCount
- Optimized port scanning with immediate shouldStop checks
- Added select() with proper timeout for connection attempts

## Version 1.0.0 - Initial Release (2026-02-23)

### Features
- Multi-threaded port scanning (1-500 threads)
- GUI version with Dear ImGui
- CLI version for automation
- Service detection
- Color-coded results
- Export to TXT format
- Preset configurations
- Real-time progress tracking
