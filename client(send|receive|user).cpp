// ============================================================
// COPY FROM MEMBER 3: UI & Messaging functions (Client Side)
// ============================================================

// Send message to another campus
void sendMessage() {
    cout << "\n=== Send Message ===\n";
    
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
    
    cout << "\nEnter your message: ";
    string messageContent;
    getline(cin, messageContent);
    
    string fullMessage = campusName + "|" + targetCampus + "|" + targetDept + "|" + messageContent;
    
    send(tcpSocket, fullMessage.c_str(), fullMessage.length(), 0);
    cout << "Message sent successfully!\n";
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

// Display menu and handle input
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
            case 1: sendMessage(); break;
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
