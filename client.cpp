#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>
#include <windows.h> // Include Windows console API header
#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool Initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// Function to set console text color
void SetConsoleColor(const string& color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    cout << color;
}

// Function to reset console text color
void ResetConsoleColor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

// Function to send messages to the server
void SendMsg(SOCKET s) {
    cout << "Enter your name: ";
    string name;
    getline(cin, name);
    string umsg;
    while (true) {
        getline(cin, umsg);
        string smsg = name + " : " + umsg;
        int bytesent = send(s, smsg.c_str(), smsg.length(), 0);
        if (bytesent == SOCKET_ERROR) {
            cout << "Error sending message" << endl;
            break;
        }
        if (umsg == "Quit") {
            cout << "Stopping the program" << endl;
            break;
        }
    }
    closesocket(s);
    WSACleanup();
}

// Function to receive messages from the server
void RecMsg(SOCKET s) {
    char buffer[4096];
    int recvlength;

    while (true) {
        recvlength = recv(s, buffer, sizeof(buffer), 0);
        if (recvlength <= 0) {
            cout << "Disconnected from the server" << endl;
            break;
        } else {
            // Parse the received message to extract color and content
            string receivedMsg(buffer, recvlength);

            // Find the position of the first colon (which separates color and content)
            size_t colonPos = receivedMsg.find(':');
            if (colonPos != string::npos) {
                string colorStr = receivedMsg.substr(0, colonPos);
                string contentStr = receivedMsg.substr(colonPos + 1);

                // Set console text color based on the received color
                SetConsoleColor(colorStr);
                // Display the content of the message
                cout << contentStr << endl;
                ResetConsoleColor(); // Reset color after displaying message
            }
        }
    }

    closesocket(s);
    WSACleanup();
}

int main() {
    // Initialize Winsock
    if (!Initialize()) {
        cout << "Initialize Winsock Failed" << endl;
        return 0;
    }

    // Create socket
    SOCKET s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        cout << "Invalid socket created" << endl;
        return 0;
    }

    // Set server details
    int port = 12345;
    string serveraddress = "127.0.0.1";
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Set localhost address

    // Connect to server
    if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR) {
        cout << "Not able to connect to server" << endl;
        cout << ":" << WSAGetLastError();
        closesocket(s);
        WSACleanup();
        return 1;
    }

    // Connection successful
    cout << "Successfully connected to server" << endl;

    // Create threads for sending and receiving messages
    thread receiverThread(RecMsg, s);
    thread senderThread(SendMsg, s);

    // Wait for threads to finish
    senderThread.join();
    receiverThread.join();

    return 0;
}
