#include <iostream>
#include <thread>
#include <mutex>
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

// Configuration
const string SERVER_IP = "127.0.0.1";
const int TCP_PORT = 8080;
const int UDP_PORT = 8081;
const int CLIENT_UDP_PORT = 8082;
const int HEARTBEAT_INTERVAL = 10; // seconds

// Global variables
SOCKET tcpSocket = INVALID_SOCKET;
SOCKET udpSocket = INVALID_SOCKET;
string campusName;
string campusPassword;
bool isConnected = false;
mutex coutMutex;

// Department list
const string departments[] = {"CS", "SE", "AI", "EE"};

void safeLog(const string& message) {
    lock_guard<mutex> lock(coutMutex);
    cout << message << endl;
}

// Connect to central server via TCP
bool connectToServer() {
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket == INVALID_SOCKET) {
        safeLog("Failed to create TCP socket");
        return false;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(TCP_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr);
    
    if (connect(tcpSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        safeLog("Failed to connect to server");
        closesocket(tcpSocket);
        return false;
    }
    
    // Send authentication
    string authMsg = "Campus:" + campusName + ",Pass:" + campusPassword;
    send(tcpSocket, authMsg.c_str(), authMsg.length(), 0);
    
    // Wait for response
    char buffer[1024] = {0};
    int bytesReceived = recv(tcpSocket, buffer, sizeof(buffer), 0);
    
    if (bytesReceived > 0) {
        string response(buffer, bytesReceived);
        if (response == "AUTH_SUCCESS") {
            safeLog("Authentication successful!");
            isConnected = true;
            return true;
        } else {
            safeLog("Authentication failed!");
        }
    }
    
    closesocket(tcpSocket);
    return false;
}

// UDP heartbeat sender
void sendHeartbeat() {
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == INVALID_SOCKET) {
        safeLog("Failed to create UDP socket for heartbeat");
        return;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(UDP_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serverAddr.sin_addr);
    
    while (isConnected) {
        string heartbeat = "HEARTBEAT|" + campusName;
        sendto(udpSocket, heartbeat.c_str(), heartbeat.length(), 0,
               (sockaddr*)&serverAddr, sizeof(serverAddr));
        
        safeLog("[Heartbeat] Status sent to server");
        this_thread::sleep_for(chrono::seconds(HEARTBEAT_INTERVAL));
    }
    
    closesocket(udpSocket);
}

// UDP broadcast receiver
void receiveBroadcasts() {
    SOCKET broadcastSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (broadcastSocket == INVALID_SOCKET) {
        safeLog("Failed to create UDP socket for broadcasts");
        return;
    }
    
    sockaddr_in clientAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = INADDR_ANY;
    clientAddr.sin_port = htons(CLIENT_UDP_PORT);
    
    if (bind(broadcastSocket, (sockaddr*)&clientAddr, sizeof(clientAddr)) == SOCKET_ERROR) {
        safeLog("Failed to bind UDP socket for broadcasts");
        closesocket(broadcastSocket);
        return;
    }
    
    char buffer[4096];
    while (isConnected) {
        sockaddr_in serverAddr;
        socklen_t serverLen = sizeof(serverAddr);
        
        int bytesReceived = recvfrom(broadcastSocket, buffer, sizeof(buffer), 0,
                                     (sockaddr*)&serverAddr, &serverLen);
        
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            string message(buffer);
            
            if (message.find("BROADCAST|") == 0) {
                string content = message.substr(10);
                safeLog("\n*** SYSTEM BROADCAST ***");
                safeLog(content);
                safeLog("************************\n");
            }
        }
    }
    
    closesocket(broadcastSocket);
}

// TCP message receiver
void receiveMessages() {
    char buffer[4096];
    
    while (isConnected) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(tcpSocket, buffer, sizeof(buffer), 0);
        
        if (bytesReceived <= 0) {
            safeLog("Disconnected from server");
            isConnected = false;
            break;
        }
        
        string message(buffer, bytesReceived);
        
        if (message.find("ERROR|") == 0) {
            safeLog("\n[ERROR] " + message.substr(6));
        } else {
            // Format: FROM|DEPT|MESSAGE
            size_t pos1 = message.find('|');
            size_t pos2 = message.find('|', pos1 + 1);
            
            if (pos1 != string::npos && pos2 != string::npos) {
                string from = message.substr(0, pos1);
                string dept = message.substr(pos1 + 1, pos2 - pos1 - 1);
                string content = message.substr(pos2 + 1);
                
                safeLog("\n╔════════════════════════════════════════╗");
                safeLog("║         NEW MESSAGE RECEIVED           ║");
                safeLog("╠════════════════════════════════════════╣");
                safeLog("║ From: " + from + " (" + dept + ")");
                safeLog("║ Message: " + content);
                safeLog("╚════════════════════════════════════════╝\n");
            }
        }
    }
}

// Send message to another campus
void sendMessage() {
    cout << "\n=== Send Message ===\n";
    
    // Select target campus
    cout << "Available Campuses:\n";
    cout << "1. Lahore\n";
    cout << "2. Karachi\n";
    cout << "3. Peshawar\n";
    cout << "4. Chiniot\n";
    cout << "5. Multan\n";
    cout << "Enter target campus number: ";
    
    int campusChoice;
    cin >> campusChoice;
    cin.ignore();
    
    string targetCampus;
    switch(campusChoice) {
        case 1: targetCampus = "Lahore"; break;
        case 2: targetCampus = "Karachi"; break;
        case 3: targetCampus = "Peshawar"; break;
        case 4: targetCampus = "Chiniot"; break;
        case 5: targetCampus = "Multan"; break;
        default:
            cout << "Invalid choice!\n";
            return;
    }
    
    if (targetCampus == campusName) {
        cout << "Cannot send message to yourself!\n";
        return;
    }
    
    // Select department
    cout << "\nSelect Target Department:\n";
    for (int i = 0; i < 4; i++) {
        cout << (i + 1) << ". " << departments[i] << "\n";
    }
    cout << "Enter department number: ";
    
    int deptChoice;
    cin >> deptChoice;
    cin.ignore();
    
    if (deptChoice < 1 || deptChoice > 4) {
        cout << "Invalid department!\n";
        return;
    }
    
    string targetDept = departments[deptChoice - 1];
    
    // Get message
    cout << "\nEnter your message: ";
    string messageContent;
    getline(cin, messageContent);
    
    // Format: FROM|TO|DEPT|MESSAGE
    string fullMessage = campusName + "|" + targetCampus + "|" + targetDept + "|" + messageContent;
    
    send(tcpSocket, fullMessage.c_str(), fullMessage.length(), 0);
    cout << "Message sent successfully!\n";
}

// Display menu and handle user input
void userInterface() {
    while (isConnected) {
        cout << "\n╔════════════════════════════════════════╗\n";
        cout << "║  " << campusName << " Campus - NU Info Exchange  \n";
        cout << "╠════════════════════════════════════════╣\n";
        cout << "║  1. Send Message to Another Campus     ║\n";
        cout << "║  2. View Connection Status             ║\n";
        cout << "║  3. Exit                               ║\n";
        cout << "╚════════════════════════════════════════╝\n";
        cout << "Enter your choice: ";
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        switch(choice) {
            case 1:
                sendMessage();
                break;
            case 2:
                cout << "\nConnection Status: " << (isConnected ? "Connected" : "Disconnected") << "\n";
                cout << "Campus: " << campusName << "\n";
                cout << "Server: " << SERVER_IP << ":" << TCP_PORT << "\n";
                break;
            case 3:
                cout << "Disconnecting...\n";
                isConnected = false;
                return;
            default:
                cout << "Invalid choice!\n";
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
    cout << "  Campus Client\n";
    cout << "======================================\n\n";
    
    // Campus selection
    cout << "Select your campus:\n";
    cout << "1. Lahore (Password: 23L-0999)\n";
    cout << "2. Karachi (Password: 23K-0664)\n";
    cout << "3. Peshawar (Password: 23P-0871)\n";
    cout << "4. Chiniot/CFD (Password: 23F-0763)\n";
    cout << "5. Multan (Password: 23M-0740)\n";
    cout << "Enter campus number (1-5): ";
    
    int choice;
    if (!(cin >> choice)) {
        cout << "\nError: Please enter a NUMBER (1-5), not text!\n";
        cout << "Example: Enter '1' for Lahore\n";
        return 1;
    }
    cin.ignore();
    
    switch(choice) {
        case 1: campusName = "Lahore"; campusPassword = "23L-0999"; break;
        case 2: campusName = "Karachi"; campusPassword = "23K-0664"; break;
        case 3: campusName = "Peshawar"; campusPassword = "23P-0871"; break;
        case 4: campusName = "Chiniot"; campusPassword = "23F-0763"; break;
        case 5: campusName = "Multan"; campusPassword = "23M-0740"; break;
        default:
            cout << "\nInvalid choice! Please enter a number between 1 and 5.\n";
            cout << "You entered: " << choice << "\n";
            cout << "Try again and enter just the number (e.g., '1' for Lahore)\n";
            return 1;
    }
    
    cout << "\nConnecting to Central Server as " << campusName << " campus...\n";
    
    if (!connectToServer()) {
        cout << "Failed to connect to server!\n";
        return 1;
    }
    
    // Start background threads
    thread heartbeatThread(sendHeartbeat);
    thread broadcastThread(receiveBroadcasts);
    thread receiveThread(receiveMessages);
    
    // Run user interface in main thread
    userInterface();
    
    // Cleanup
    heartbeatThread.join();
    broadcastThread.join();
    receiveThread.join();
    
    closesocket(tcpSocket);
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    return 0;
}
