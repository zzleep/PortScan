#ifndef PORT_SCANNER_H
#define PORT_SCANNER_H

#include <string>
#include <vector>
#include <map>
#include <mutex>

class PortScanner {
private:
    std::string target;
    std::string ip;
    std::vector<int> ports;
    int numThreads;
    int timeout;
    std::vector<std::pair<int, std::string>> openPorts;
    std::mutex portsMutex;
    std::mutex progressMutex;
    int portsScanned;
    
    // Helper functions
    bool resolveHostname();
    void scanPort(int port);
    std::string getServiceName(int port);
    
public:
    PortScanner(const std::string& target, const std::vector<int>& ports, 
                int threads = 100, int timeout = 1);
    
    void scan();
    void printResults(double duration);
};

#endif // PORT_SCANNER_H
