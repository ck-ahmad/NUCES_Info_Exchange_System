#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <cstring>
#include <chrono>
#include <sstream>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

using namespace std;

// Port configurations
const int TCP_PORT = 8080;
const int UDP_PORT = 8081;

// Campus credentials (Campus:Password)
map<string, string> campusCredentials = {
    {"Lahore", "23L-0999"},
    {"Karachi", "23K-0664"},
    {"Peshawar", "23P-0871"},
    {"Chiniot", "23F-0763"},
    {"Multan", "23M-0740"}
};

// Connected clients structure
struct CampusClient {
    SOCKET socket;
    string campusName;
    string lastSeen;
    bool isOnline;
};

map<string, CampusClient> connectedClients;
mutex clientsMutex;
mutex coutMutex;

// Function to get current timestamp
string getCurrentTime() {
    time_t now = time(0);
    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

// Thread-safe console output
void safeLog(const string& message) {
    lock_guard<mutex> lock(coutMutex);
    cout << "[" << getCurrentTime() << "] " << message << endl;
}

// Parse message format: FROM|TO|DEPT|MESSAGE
struct Message {
    string from;
    string to;
    string dept;
    string content;
};

Message parseMessage(const string& msg) {
    Message m;
    stringstream ss(msg);
    getline(ss, m.from, '|');
    getline(ss, m.to, '|');
    getline(ss, m.dept, '|');
    getline(ss, m.content);
    return m;
}

// Handle authentication
bool authenticateCampus(SOCKET clientSocket, string& campusName) {
    char buffer[1024] = {0};
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    
    if (bytesReceived <= 0) return false;
    
    string authMsg(buffer);
    size_t campusPos = authMsg.find("Campus:");
    size_t passPos = authMsg.find(",Pass:");
    
    if (campusPos == string::npos || passPos == string::npos) return false;
    
    campusName = authMsg.substr(campusPos + 7, passPos - campusPos - 7);
    string password = authMsg.substr(passPos + 6);
    
    // Validate credentials
    if (campusCredentials.find(campusName) != campusCredentials.end() &&
        campusCredentials[campusName] == password) {
        
        string response = "AUTH_SUCCESS";
        send(clientSocket, response.c_str(), response.length(), 0);
        return true;
    }
    
    string response = "AUTH_FAILED";
    send(clientSocket, response.c_str(), response.length(), 0);
    return false;
}

// Handle individual campus client
void handleCampusClient(SOCKET clientSocket, string campusName) {
    safeLog("Campus " + campusName + " connected successfully");
    
    char buffer[4096];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        if (bytesReceived <= 0) {
            safeLog("Campus " + campusName + " disconnected");
            break;
        }
        
        string receivedMsg(buffer, bytesReceived);
        safeLog("Message from " + campusName + ": " + receivedMsg);
        
        // Parse and route message
        Message msg = parseMessage(receivedMsg);
        
        {
            lock_guard<mutex> lock(clientsMutex);
            auto it = connectedClients.find(msg.to);
            
            if (it != connectedClients.end() && it->second.isOnline) {
                string forwardMsg = msg.from + "|" + msg.dept + "|" + msg.content;
                send(it->second.socket, forwardMsg.c_str(), forwardMsg.length(), 0);
                safeLog("Routed message from " + msg.from + " to " + msg.to);
            } else {
                string errorMsg = "ERROR|Campus " + msg.to + " is not online";
                send(clientSocket, errorMsg.c_str(), errorMsg.length(), 0);
                safeLog("Failed to route: " + msg.to + " is offline");
            }
        }
    }
    
    // Cleanup
    {
        lock_guard<mutex> lock(clientsMutex);
        connectedClients[campusName].isOnline = false;
    }
    closesocket(clientSocket);
}

// TCP Server for handling campus connections
void tcpServer() {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        safeLog("Failed to create TCP socket");
        return;
    }
    
    // Allow port reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(TCP_PORT);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        safeLog("TCP Bind failed");
        closesocket(serverSocket);
        return;
    }
    
    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        safeLog("TCP Listen failed");
        closesocket(serverSocket);
        return;
    }
    
    safeLog("TCP Server listening on port " + to_string(TCP_PORT));
    
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) continue;
        
        // Authenticate client
        string campusName;
        if (authenticateCampus(clientSocket, campusName)) {
            {
                lock_guard<mutex> lock(clientsMutex);
                connectedClients[campusName] = {clientSocket, campusName, getCurrentTime(), true};
            }
            
            // Handle client in new thread
            thread(handleCampusClient, clientSocket, campusName).detach();
        } else {
            safeLog("Authentication failed for a client");
            closesocket(clientSocket);
        }
    }
    
    closesocket(serverSocket);
}

// UDP Server for heartbeat monitoring
void udpServer() {
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == INVALID_SOCKET) {
        safeLog("Failed to create UDP socket");
        return;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(UDP_PORT);
    
    if (bind(udpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        safeLog("UDP Bind failed");
        closesocket(udpSocket);
        return;
    }
    
    safeLog("UDP Server listening on port " + to_string(UDP_PORT));
    
    char buffer[1024];
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        
        int bytesReceived = recvfrom(udpSocket, buffer, sizeof(buffer), 0,
                                     (sockaddr*)&clientAddr, &clientLen);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            string heartbeat(buffer);
            
            // Expected format: "HEARTBEAT|CampusName"
            if (heartbeat.find("HEARTBEAT|") == 0) {
                string campusName = heartbeat.substr(10);
                
                lock_guard<mutex> lock(clientsMutex);
                if (connectedClients.find(campusName) != connectedClients.end()) {
                    connectedClients[campusName].lastSeen = getCurrentTime();
                    safeLog("Heartbeat received from " + campusName);
                }
            }
        }
    }
    
    closesocket(udpSocket);
}

// Admin console for monitoring and broadcasting
void adminConsole() {
    safeLog("Admin Console started. Type 'help' for commands.");
    
    string command;
    while (true) {
        cout << "\nAdmin> ";
        getline(cin, command);
        
        if (command == "help") {
            cout << "Commands:\n"
                 << "  status  - Show all connected campuses\n"
                 << "  broadcast <message> - Broadcast message to all campuses\n"
                 << "  quit - Exit server\n";
        }
        else if (command == "status") {
            lock_guard<mutex> lock(clientsMutex);
            cout << "\n=== Connected Campuses ===\n";
            for (const auto& pair : connectedClients) {
                cout << "Campus: " << pair.first
                     << " | Status: " << (pair.second.isOnline ? "Online" : "Offline")
                     << " | Last Seen: " << pair.second.lastSeen << "\n";
            }
        }
        else if (command.find("broadcast ") == 0) {
            string message = command.substr(10);
            
            // Create UDP socket for broadcasting
            SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
            
            lock_guard<mutex> lock(clientsMutex);
            for (const auto& pair : connectedClients) {
                if (pair.second.isOnline) {
                    sockaddr_in campusAddr;
                    campusAddr.sin_family = AF_INET;
                    campusAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                    campusAddr.sin_port = htons(UDP_PORT + 1); // Client UDP port
                    
                    string broadcastMsg = "BROADCAST|" + message;
                    sendto(udpSocket, broadcastMsg.c_str(), broadcastMsg.length(), 0,
                           (sockaddr*)&campusAddr, sizeof(campusAddr));
                }
            }
            
            closesocket(udpSocket);
            safeLog("Broadcast sent: " + message);
        }
        else if (command == "quit") {
            safeLog("Shutting down server...");
            exit(0);
        }
    }
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed" << endl;
        return 1;
    }
#endif
    
    cout << "======================================\n";
    cout << "  NU-Information Exchange System\n";
    cout << "  Central Server (Islamabad Campus)\n";
    cout << "======================================\n\n";
    
    // Start TCP and UDP servers in separate threads
    thread tcpThread(tcpServer);
    thread udpThread(udpServer);
    
    // Run admin console in main thread
    adminConsole();
    
    tcpThread.join();
    udpThread.join();
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}
