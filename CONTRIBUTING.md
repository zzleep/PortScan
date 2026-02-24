# Contributing to PortScan

Thank you for your interest in contributing to PortScan! 🎉

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn and grow

## How to Contribute

### Reporting Bugs

1. Check if the bug already exists in [Issues](https://github.com/yourusername/PortScan/issues)
2. Create a new issue with:
   - Clear title and description
   - Steps to reproduce
   - Expected vs actual behavior
   - Your environment (OS, compiler version)

### Suggesting Features

1. Open an issue with the `enhancement` label
2. Describe the feature and use case
3. Explain why it would be useful

### Pull Requests

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Make your changes
4. Test thoroughly
5. Commit with clear messages: `git commit -m "Add amazing feature"`
6. Push to your fork: `git push origin feature/amazing-feature`
7. Open a Pull Request

### Code Style

- Use consistent indentation (4 spaces)
- Follow C++17 standards
- Add comments for complex logic
- Keep functions focused and small

### Testing

- Test on both Windows and Linux (if possible)
- Verify multi-threading works correctly
- Check for memory leaks
- Test with various network conditions

## Development Setup

### Prerequisites
- GCC/MinGW-w64 with C++17 support
- Git

### Building

**CLI Version:**
```bash
cd cli
./build.sh  # Linux
build.bat   # Windows
```

**GUI Version:**
```bash
cd gui_imgui
./build_imgui.sh  # Will download ImGui automatically
```

## Questions?

Feel free to open an issue for any questions!

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
