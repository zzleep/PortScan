#include "PortScanner.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <string>

// Function prototypes
std::vector<int> parsePorts(const std::string& portString);
void printUsage(const char* programName);
std::vector<int> getCommonPorts();

int main(int argc, char* argv[]) {
    // Default values
    std::string target;
    std::string portString = "1-1024";
    int threads = 100;
    int timeout = 1;
    bool useCommonPorts = false;
    
    // Parse command-line arguments
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }
    
    target = argv[1];
    
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        
        if ((arg == "-p" || arg == "--ports") && i + 1 < argc) {
            portString = argv[++i];
        }
        else if ((arg == "-t" || arg == "--threads") && i + 1 < argc) {
            threads = std::stoi(argv[++i]);
        }
        else if (arg == "--timeout" && i + 1 < argc) {
            timeout = std::stoi(argv[++i]);
        }
        else if (arg == "--common") {
            useCommonPorts = true;
        }
        else if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        else {
            std::cerr << "[!] Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }
    
    std::vector<int> ports;
    
    if (useCommonPorts) {
        ports = getCommonPorts();
    } else {
        ports = parsePorts(portString);
    }
    
    if (ports.empty()) {
        std::cerr << "[!] Error: No valid ports to scan" << std::endl;
        return 1;
    }
    
    // Validate port range
    for (int port : ports) {
        if (port < 1 || port > 65535) {
            std::cerr << "[!] Error: Port numbers must be between 1 and 65535" << std::endl;
            return 1;
        }
    }
    
    try {
        PortScanner scanner(target, ports, threads, timeout);
        scanner.scan();
    }
    catch (const std::exception& e) {
        std::cerr << "\n[!] Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

std::vector<int> parsePorts(const std::string& portString) {
    std::vector<int> ports;
    std::stringstream ss(portString);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        size_t dashPos = token.find('-');
        
        if (dashPos != std::string::npos) {
            // Port range
            int start = std::stoi(token.substr(0, dashPos));
            int end = std::stoi(token.substr(dashPos + 1));
            
            if (start > end) {
                std::swap(start, end);
            }
            
            for (int port = start; port <= end; ++port) {
                ports.push_back(port);
            }
        } else {
            // Single port
            ports.push_back(std::stoi(token));
        }
    }
    
    // Remove duplicates and sort
    std::sort(ports.begin(), ports.end());
    ports.erase(std::unique(ports.begin(), ports.end()), ports.end());
    
    return ports;
}

std::vector<int> getCommonPorts() {
    return {
        20, 21, 22, 23, 25, 53, 80, 110, 111, 135, 139, 143, 443, 445,
        993, 995, 1723, 3306, 3389, 5432, 5900, 8080, 8443
    };
}

void printUsage(const char* programName) {
    std::cout << "Sophisticated Multi-threaded Port Scanner\n" << std::endl;
    std::cout << "Usage: " << programName << " <target> [options]\n" << std::endl;
    std::cout << "Required:" << std::endl;
    std::cout << "  <target>              Target IP address or hostname\n" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -p, --ports <ports>   Ports to scan (default: 1-1024)" << std::endl;
    std::cout << "                        Examples: 80, 1-1000, 80,443,8080" << std::endl;
    std::cout << "  -t, --threads <num>   Number of threads (default: 100)" << std::endl;
    std::cout << "  --timeout <seconds>   Socket timeout in seconds (default: 1)" << std::endl;
    std::cout << "  --common              Scan common ports only (faster)" << std::endl;
    std::cout << "  -h, --help            Display this help message\n" << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " 192.168.1.1 -p 1-1000" << std::endl;
    std::cout << "  " << programName << " example.com -p 80,443,8080" << std::endl;
    std::cout << "  " << programName << " 10.0.0.1 -p 1-65535 -t 200" << std::endl;
    std::cout << "  " << programName << " scanme.nmap.org --common" << std::endl;
}
