#include "PortScannerImGui.h"
#include "imgui.h"
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <map>
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
    #define closesocket close
#endif

PortScannerImGui::PortScannerImGui() 
    : threadsCount(100), timeoutSeconds(1), selectedPreset(0), autoScroll(true),
      showAboutPopup(false), scanActive(false), shouldStop(false), stopComplete(false),
      totalPorts(0), scannedPorts(0), scanThread(nullptr), scanDuration(0.0), openPortsCount(0) {
    
    strcpy(targetInput, "");
    strcpy(portsInput, "1-1024");
    
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

PortScannerImGui::~PortScannerImGui() {
    if (scanThread) {
        shouldStop = true;
        // Wait with timeout for graceful shutdown
        auto start = std::chrono::steady_clock::now();
        while (!stopComplete && 
               std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::steady_clock::now() - start).count() < 3) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        if (scanThread->joinable()) {
            scanThread->join();
        }
        delete scanThread;
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
}

void PortScannerImGui::render() {
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);
    
    ImGui::Begin("Advanced Port Scanner", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Export Results", "Ctrl+S")) {
                exportResults();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Handle exit
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Clear Results", "Ctrl+L")) {
                clearResults();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                showAboutPopup = true;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    // About popup
    if (showAboutPopup) {
        ImGui::OpenPopup("About");
        showAboutPopup = false;
    }
    
    if (ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Advanced Port Scanner v1.0");
        ImGui::Separator();
        ImGui::Text("A sophisticated multi-threaded port scanner");
        ImGui::Text("Built with Dear ImGui & C++17");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), 
            "Features:");
        ImGui::BulletText("Multi-threaded scanning (up to 500 threads)");
        ImGui::BulletText("Service detection");
        ImGui::BulletText("Color-coded results");
        ImGui::BulletText("Export to TXT/CSV");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), 
            "Use responsibly and only on authorized systems!");
        ImGui::Spacing();
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    
    // Main content
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 6));
    
    renderConfigSection();
    ImGui::Spacing();
    renderControlButtons();
    ImGui::Spacing();
    renderProgressSection();
    ImGui::Spacing();
    renderResultsTable();
    ImGui::Spacing();
    renderStatistics();
    
    ImGui::PopStyleVar();
    ImGui::End();
}

void PortScannerImGui::renderConfigSection() {
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.25f, 0.3f, 1.0f));
    
    if (ImGui::CollapsingHeader("Scan Configuration", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        // Target
        ImGui::Text("Target:");
        ImGui::SameLine(150);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##target", "Enter IP or hostname (e.g., 192.168.1.1)", 
            targetInput, sizeof(targetInput));
        
        // Preset
        ImGui::Text("Preset:");
        ImGui::SameLine(150);
        ImGui::SetNextItemWidth(-1);
        const char* presets[] = {
            "Custom",
            "Quick Scan (Common Ports)",
            "Standard Scan (1-1024)",
            "Full Scan (1-65535)",
            "Web Services",
            "Database Ports"
        };
        if (ImGui::Combo("##preset", &selectedPreset, presets, IM_ARRAYSIZE(presets))) {
            applyPreset(selectedPreset);
        }
        
        // Ports
        ImGui::Text("Ports:");
        ImGui::SameLine(150);
        ImGui::SetNextItemWidth(-1);
        ImGui::InputTextWithHint("##ports", "e.g., 80 or 1-1000 or 80,443,8080", 
            portsInput, sizeof(portsInput));
        
        // Threads
        ImGui::Text("Threads:");
        ImGui::SameLine(150);
        ImGui::SetNextItemWidth(150);
        ImGui::SliderInt("##threads", &threadsCount, 1, 500);
        
        // Timeout
        ImGui::Text("Timeout:");
        ImGui::SameLine(150);
        ImGui::SetNextItemWidth(150);
        ImGui::SliderInt("##timeout", &timeoutSeconds, 1, 10);
        ImGui::SameLine();
        ImGui::Text("seconds");
        
        // Options
        ImGui::Checkbox("Auto-scroll results", &autoScroll);
        
        ImGui::Unindent();
    }
    
    ImGui::PopStyleColor();
}

void PortScannerImGui::renderControlButtons() {
    float buttonWidth = (ImGui::GetContentRegionAvail().x - 30) / 4;
    
    // Start button
    if (!scanActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.13f, 0.59f, 0.95f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.10f, 0.47f, 0.82f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.05f, 0.28f, 0.64f, 1.0f));
        if (ImGui::Button("Start Scan", ImVec2(buttonWidth, 40))) {
            startScan();
        }
        ImGui::PopStyleColor(3);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        ImGui::Button("Scanning...", ImVec2(buttonWidth, 40));
        ImGui::PopStyleColor();
    }
    
    ImGui::SameLine();
    
    // Stop button
    if (scanActive && !shouldStop) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.4f, 0.4f, 1.0f));
        if (ImGui::Button("Stop", ImVec2(buttonWidth, 40))) {
            stopScan();
        }
        ImGui::PopStyleColor(2);
    } else if (scanActive && shouldStop) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.6f, 0.2f, 1.0f));
        ImGui::Button("Stopping...", ImVec2(buttonWidth, 40));
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::BeginDisabled();
        ImGui::Button("Stop", ImVec2(buttonWidth, 40));
        ImGui::EndDisabled();
        ImGui::PopStyleColor();
    }
    
    ImGui::SameLine();
    
    // Export button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.8f, 0.4f, 1.0f));
    if (ImGui::Button("Export", ImVec2(buttonWidth, 40))) {
        exportResults();
    }
    ImGui::PopStyleColor(2);
    
    ImGui::SameLine();
    
    // Clear button (disabled during scan)
    if (!scanActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.5f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.6f, 0.3f, 1.0f));
        if (ImGui::Button("Clear", ImVec2(buttonWidth, 40))) {
            clearResults();
        }
        ImGui::PopStyleColor(2);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        ImGui::BeginDisabled();
        ImGui::Button("Clear", ImVec2(buttonWidth, 40));
        ImGui::EndDisabled();
        ImGui::PopStyleColor();
    }
}

void PortScannerImGui::renderProgressSection() {
    if (ImGui::CollapsingHeader("Progress", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        // Progress bar
        float progress = totalPorts > 0 ? (float)scannedPorts / totalPorts : 0.0f;
        char progressText[128];
        snprintf(progressText, sizeof(progressText), "%d/%d ports (%.1f%%)", 
            scannedPorts.load(), totalPorts.load(), progress * 100.0f);
        
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.7f, 0.3f, 1.0f));
        ImGui::ProgressBar(progress, ImVec2(-1, 30), progressText);
        ImGui::PopStyleColor();
        
        // Status
        if (scanActive) {
            auto now = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - scanStartTime).count();
            int mins = elapsed / 60;
            int secs = elapsed % 60;
            ImGui::TextColored(ImVec4(0.13f, 0.59f, 0.95f, 1.0f), 
                "Scanning... Elapsed: %02d:%02d", mins, secs);
        } else if (scannedPorts > 0) {
            int mins = (int)scanDuration / 60;
            int secs = (int)scanDuration % 60;
            ImGui::TextColored(ImVec4(0.3f, 0.8f, 0.3f, 1.0f), 
                "Scan completed in %02d:%02d - Found %d open port(s)", 
                mins, secs, openPortsCount);
        } else {
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Ready to scan");
        }
        
        ImGui::Unindent();
    }
}

void PortScannerImGui::renderResultsTable() {
    if (ImGui::CollapsingHeader("Open Ports", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Indent();
        
        // Table
        ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | 
                                ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable |
                                ImGuiTableFlags_ScrollY;
        
        if (ImGui::BeginTable("ResultsTable", 3, flags, ImVec2(-1, 300))) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableSetupColumn("Port", ImGuiTableColumnFlags_WidthFixed, 80);
            ImGui::TableSetupColumn("Service", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Time Found", ImGuiTableColumnFlags_WidthFixed, 120);
            ImGui::TableHeadersRow();
            
            // Table content
            std::lock_guard<std::mutex> lock(portsMutex);
            for (size_t i = 0; i < openPorts.size(); i++) {
                const auto& port = openPorts[i];
                
                ImGui::TableNextRow();
                
                // Color coding
                ImVec4 rowColor = ImVec4(0.2f, 0.2f, 0.2f, 0.5f);
                if (port.service.find("http") != std::string::npos) {
                    rowColor = ImVec4(1.0f, 0.7f, 0.3f, 0.3f); // Orange for web
                } else if (port.service.find("ssh") != std::string::npos) {
                    rowColor = ImVec4(0.3f, 1.0f, 0.3f, 0.3f); // Green for SSH
                } else if (port.service.find("sql") != std::string::npos || 
                           port.service.find("database") != std::string::npos) {
                    rowColor = ImVec4(0.3f, 0.5f, 1.0f, 0.3f); // Blue for DB
                }
                
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(rowColor));
                
                // Port
                ImGui::TableNextColumn();
                ImGui::Text("%d", port.port);
                
                // Service
                ImGui::TableNextColumn();
                ImGui::Text("%s", port.service.c_str());
                
                // Time
                ImGui::TableNextColumn();
                ImGui::Text("%s", port.timeFound.c_str());
            }
            
            // Auto-scroll
            if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
            }
            
            ImGui::EndTable();
        }
        
        ImGui::Unindent();
    }
}

void PortScannerImGui::renderStatistics() {
    std::lock_guard<std::mutex> lock(portsMutex);
    ImGui::TextColored(ImVec4(0.13f, 0.59f, 0.95f, 1.0f), 
        "Total Open Ports Found: %d", (int)openPorts.size());
}

void PortScannerImGui::startScan() {
    if (scanActive) return;
    
    std::string target = targetInput;
    std::string portsStr = portsInput;
    
    if (target.empty()) {
        // Show error - target required
        return;
    }
    
    std::vector<int> ports = parsePorts(portsStr);
    if (ports.empty()) {
        // Show error - invalid ports
        return;
    }
    
    // Clear previous results
    {
        std::lock_guard<std::mutex> lock(portsMutex);
        openPorts.clear();
    }
    
    openPortsCount = 0;
    scannedPorts = 0;
    totalPorts = ports.size();
    shouldStop = false;
    scanActive = true;
    scanStartTime = std::chrono::high_resolution_clock::now();
    
    // Start scan thread
    if (scanThread) {
        if (scanThread->joinable()) {
            scanThread->join();
        }
        delete scanThread;
    }
    
    scanThread = new std::thread(&PortScannerImGui::scanThreadFunc, this, 
                                 target, ports, threadsCount, timeoutSeconds);
}

void PortScannerImGui::stopScan() {
    if (!shouldStop) {
        shouldStop = true;
        stopComplete = false;
    }
}

void PortScannerImGui::scanThreadFunc(const std::string& target, 
                                      const std::vector<int>& ports,
                                      int threads, int timeout) {
    std::string ip = resolveHostname(target);
    if (ip.empty()) {
        scanActive = false;
        return;
    }
    
    // Create thread pool
    std::vector<std::thread> threadPool;
    size_t portsPerThread = (ports.size() + threads - 1) / threads;
    
    for (int t = 0; t < threads && !shouldStop; ++t) {
        size_t startIdx = t * portsPerThread;
        size_t endIdx = std::min(startIdx + portsPerThread, ports.size());
        
        if (startIdx >= ports.size()) break;
        
        threadPool.emplace_back([this, &ip, &ports, startIdx, endIdx, timeout]() {
            for (size_t i = startIdx; i < endIdx && !shouldStop; ++i) {
                scanPort(ip, ports[i], timeout);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threadPool) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    auto endTime = std::chrono::high_resolution_clock::now();
    scanDuration = std::chrono::duration<double>(endTime - scanStartTime).count();
    
    {
        std::lock_guard<std::mutex> lock(portsMutex);
        openPortsCount = openPorts.size();
    }
    
    stopComplete = true;
    scanActive = false;
}

void PortScannerImGui::scanPort(const std::string& ip, int port, int timeout) {
    if (shouldStop) {
        scannedPorts++;
        return;
    }
    
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        scannedPorts++;
        return;
    }
    
    // Set non-blocking mode for faster timeout response
#ifdef _WIN32
    unsigned long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    
    // Start non-blocking connect
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    // Use select for timeout
    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);
    
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    
    int selectResult = select(sock + 1, NULL, &writefds, NULL, &tv);
    
    if (selectResult > 0 && !shouldStop) {
        // Check if connection was successful
        int error = 0;
        socklen_t len = sizeof(error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&error, &len);
        
        if (error == 0) {
            OpenPort op;
            op.port = port;
            op.service = getServiceName(port);
            op.timeFound = getCurrentTime();
            
            std::lock_guard<std::mutex> lock(portsMutex);
            openPorts.push_back(op);
        }
    }
    
    closesocket(sock);
    scannedPorts++;
}

std::vector<int> PortScannerImGui::parsePorts(const std::string& portsStr) {
    std::vector<int> ports;
    std::stringstream ss(portsStr);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        size_t dashPos = token.find('-');
        if (dashPos != std::string::npos) {
            int start = std::stoi(token.substr(0, dashPos));
            int end = std::stoi(token.substr(dashPos + 1));
            if (start > end) std::swap(start, end);
            for (int p = start; p <= end; ++p) {
                ports.push_back(p);
            }
        } else {
            ports.push_back(std::stoi(token));
        }
    }
    
    std::sort(ports.begin(), ports.end());
    ports.erase(std::unique(ports.begin(), ports.end()), ports.end());
    return ports;
}

std::string PortScannerImGui::resolveHostname(const std::string& target) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(target.c_str(), NULL, &hints, &result) != 0) {
        return "";
    }
    
    struct sockaddr_in* addr = (struct sockaddr_in*)result->ai_addr;
    std::string ip = inet_ntoa(addr->sin_addr);
    freeaddrinfo(result);
    return ip;
}

std::string PortScannerImGui::getServiceName(int port) {
    static std::map<int, std::string> services = {
        {20, "ftp-data"}, {21, "ftp"}, {22, "ssh"}, {23, "telnet"},
        {25, "smtp"}, {53, "dns"}, {80, "http"}, {110, "pop3"},
        {143, "imap"}, {443, "https"}, {445, "microsoft-ds"},
        {3306, "mysql"}, {3389, "rdp"}, {5432, "postgresql"},
        {5900, "vnc"}, {8080, "http-proxy"}, {8443, "https-alt"}
    };
    
    auto it = services.find(port);
    return (it != services.end()) ? it->second : "unknown";
}

std::string PortScannerImGui::getCurrentTime() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm);
    return std::string(buffer);
}

void PortScannerImGui::applyPreset(int preset) {
    switch (preset) {
        case 1: // Quick Scan
            strcpy(portsInput, "20-25,80,110,143,443,445,3306,3389,5900,8080");
            break;
        case 2: // Standard
            strcpy(portsInput, "1-1024");
            break;
        case 3: // Full
            strcpy(portsInput, "1-65535");
            threadsCount = 200;
            break;
        case 4: // Web
            strcpy(portsInput, "80,443,8000,8080,8443,8888");
            break;
        case 5: // Database
            strcpy(portsInput, "1433,3306,5432,6379,27017");
            break;
    }
}

void PortScannerImGui::exportResults() {
    if (openPorts.empty()) return;
    
    std::lock_guard<std::mutex> lock(portsMutex);
    
    std::time_t now = std::time(nullptr);
    char filename[128];
    std::strftime(filename, sizeof(filename), "scan_results_%Y%m%d_%H%M%S.txt", std::localtime(&now));
    
    std::ofstream file(filename);
    if (!file.is_open()) return;
    
    file << "Port Scanner Results\n";
    file << "Target: " << targetInput << "\n";
    file << "Total Open Ports: " << openPorts.size() << "\n";
    file << std::string(60, '=') << "\n\n";
    file << "Port     Service                Time Found\n";
    file << std::string(60, '-') << "\n";
    
    for (const auto& port : openPorts) {
        file << std::setw(8) << std::left << port.port
             << std::setw(23) << port.service
             << port.timeFound << "\n";
    }
    
    file.close();
}

void PortScannerImGui::clearResults() {
    if (!scanActive) {
        std::lock_guard<std::mutex> lock(portsMutex);
        openPorts.clear();
        scannedPorts = 0;
        totalPorts = 0;
        openPortsCount = 0;
        scanDuration = 0.0;
    }
}
