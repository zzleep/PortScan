#include "PortScanner.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

PortScanner::PortScanner(const std::string& target, const std::vector<int>& ports,
                         int threads, int timeout)
    : target(target), ports(ports), numThreads(threads), 
      timeout(timeout), portsScanned(0) {
    
#ifdef _WIN32
    // Initialize Winsock on Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "[!] Failed to initialize Winsock" << std::endl;
        exit(1);
    }
#endif
}

bool PortScanner::resolveHostname() {
    struct addrinfo hints, *result;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(target.c_str(), NULL, &hints, &result);
    
    if (status != 0) {
        std::cerr << "[!] Error: Unable to resolve hostname '" << target << "'" << std::endl;
        return false;
    }
    
    struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;
    ip = inet_ntoa(addr->sin_addr);
    
    freeaddrinfo(result);
    return true;
}

std::string PortScanner::getServiceName(int port) {
    static std::map<int, std::string> commonServices = {
        {20, "ftp-data"}, {21, "ftp"}, {22, "ssh"}, {23, "telnet"},
        {25, "smtp"}, {53, "dns"}, {80, "http"}, {110, "pop3"},
        {111, "rpcbind"}, {135, "msrpc"}, {139, "netbios-ssn"},
        {143, "imap"}, {443, "https"}, {445, "microsoft-ds"},
        {993, "imaps"}, {995, "pop3s"}, {1723, "pptp"},
        {3306, "mysql"}, {3389, "ms-wbt-server"}, {5432, "postgresql"},
        {5900, "vnc"}, {8080, "http-proxy"}, {8443, "https-alt"}
    };
    
    auto it = commonServices.find(port);
    if (it != commonServices.end()) {
        return it->second;
    }
    
    // Try to get service name from system
    struct servent* service = getservbyport(htons(port), "tcp");
    if (service != NULL) {
        return std::string(service->s_name);
    }
    
    return "unknown";
}

void PortScanner::scanPort(int port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if (sock == INVALID_SOCKET) {
        std::lock_guard<std::mutex> lock(progressMutex);
        portsScanned++;
        return;
    }
    
    // Set socket timeout
#ifdef _WIN32
    DWORD timeoutMs = timeout * 1000;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeoutMs, sizeof(timeoutMs));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeoutMs, sizeof(timeoutMs));
#else
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    // Attempt connection
    int result = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    if (result == 0) {
        std::string service = getServiceName(port);
        
        {
            std::lock_guard<std::mutex> lock(portsMutex);
            openPorts.push_back({port, service});
        }
        
        std::cout << "[+] Port " << std::setw(5) << port << " - OPEN";
        if (service != "unknown") {
            std::cout << " (" << service << ")";
        }
        std::cout << std::endl;
    }
    
    closesocket(sock);
    
    // Update progress
    {
        std::lock_guard<std::mutex> lock(progressMutex);
        portsScanned++;
        
        if (portsScanned % 100 == 0 || portsScanned == (int)ports.size()) {
            double progress = (double)portsScanned / ports.size() * 100.0;
            std::cout << "\r[*] Progress: " << portsScanned << "/" << ports.size() 
                     << " (" << std::fixed << std::setprecision(1) << progress << "%)" 
                     << std::flush;
        }
    }
}

void PortScanner::scan() {
    if (!resolveHostname()) {
        return;
    }
    
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Starting scan on target: " << target << " (" << ip << ")" << std::endl;
    std::cout << "Scanning " << ports.size() << " ports with " << numThreads << " threads" << std::endl;
    std::cout << "Timeout: " << timeout << " seconds" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Create thread pool
    std::vector<std::thread> threads;
    size_t portsPerThread = (ports.size() + numThreads - 1) / numThreads;
    
    for (int t = 0; t < numThreads; ++t) {
        size_t startIdx = t * portsPerThread;
        size_t endIdx = std::min(startIdx + portsPerThread, ports.size());
        
        if (startIdx >= ports.size()) break;
        
        threads.emplace_back([this, startIdx, endIdx]() {
            for (size_t i = startIdx; i < endIdx; ++i) {
                scanPort(ports[i]);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;
    
    printResults(duration.count());
    
#ifdef _WIN32
    WSACleanup();
#endif
}

void PortScanner::printResults(double duration) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Scan completed in: " << std::fixed << std::setprecision(2) 
              << duration << " seconds" << std::endl;
    std::cout << "Total ports scanned: " << ports.size() << std::endl;
    std::cout << "Open ports found: " << openPorts.size() << std::endl;
    
    if (!openPorts.empty()) {
        // Sort ports
        std::sort(openPorts.begin(), openPorts.end(),
                 [](const auto& a, const auto& b) { return a.first < b.first; });
        
        std::cout << "\n" << std::string(70, '-') << std::endl;
        std::cout << "OPEN PORTS SUMMARY:" << std::endl;
        std::cout << std::string(70, '-') << std::endl;
        
        for (const auto& [port, service] : openPorts) {
            std::cout << "  Port " << std::setw(5) << port;
            if (service != "unknown") {
                std::cout << " - " << service;
            }
            std::cout << std::endl;
        }
    }
    
    std::cout << std::string(70, '=') << "\n" << std::endl;
}
