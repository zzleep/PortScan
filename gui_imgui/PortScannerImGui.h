#ifndef PORTSCANNER_IMGUI_H
#define PORTSCANNER_IMGUI_H

#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

struct OpenPort {
    int port;
    std::string service;
    std::string timeFound;
};

class PortScannerImGui {
public:
    PortScannerImGui();
    ~PortScannerImGui();
    
    void render();
    void startScan();
    void stopScan();
    bool isScanActive() const { return scanActive; }
    
private:
    // UI state
    char targetInput[256];
    char portsInput[256];
    int threadsCount;
    int timeoutSeconds;
    int selectedPreset;
    bool autoScroll;
    bool showAboutPopup;
    
    // Scan state
    std::atomic<bool> scanActive;
    std::atomic<bool> shouldStop;
    std::atomic<bool> stopComplete;
    std::atomic<int> totalPorts;
    std::atomic<int> scannedPorts;
    std::vector<OpenPort> openPorts;
    std::mutex portsMutex;
    std::thread* scanThread;
    
    // Statistics
    std::chrono::time_point<std::chrono::high_resolution_clock> scanStartTime;
    double scanDuration;
    int openPortsCount;
    
    // Internal methods
    void renderConfigSection();
    void renderControlButtons();
    void renderProgressSection();
    void renderResultsTable();
    void renderStatistics();
    
    void scanThreadFunc(const std::string& target, const std::vector<int>& ports, int threads, int timeout);
    void scanPort(const std::string& ip, int port, int timeout);
    std::vector<int> parsePorts(const std::string& portsStr);
    std::string resolveHostname(const std::string& target);
    std::string getServiceName(int port);
    std::string getCurrentTime();
    void applyPreset(int preset);
    void exportResults();
    void clearResults();
};

#endif // PORTSCANNER_IMGUI_H
