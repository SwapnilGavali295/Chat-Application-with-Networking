#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <map>
#include <random>
#include <ctime>
#include <tchar.h>
#pragma comment(lib, "libws2_32.a")

using namespace std;

// ANSI color escape codes for different colors
const string ANSI_COLOR_RED = "\033[1;31m";
const string ANSI_COLOR_GREEN = "\033[1;32m";
const string ANSI_COLOR_YELLOW = "\033[1;33m";
const string ANSI_COLOR_BLUE = "\033[1;34m";
const string ANSI_COLOR_RESET = "\033[0m"; // Reset color to default

// Function to initialize Winsock library
bool Initialize() {
    WSADATA data;
    return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

// Function to generate a random color from available colors
string GetRandomColor() {
    static const vector<string> colors = {ANSI_COLOR_RED, ANSI_COLOR_GREEN, ANSI_COLOR_YELLOW, ANSI_COLOR_BLUE};
    static mt19937 rng(time(nullptr));
    uniform_int_distribution<int> dist(0, colors.size() - 1);
    return colors[dist(rng)];
}

// Function to interact with a client
void InteractWithClient(SOCKET clientSocket, vector<SOCKET>& clients, map<SOCKET, string>& clientColors) {
    cout << "Client Connected" << endl;

    // Choose a random color for the client
    string clientColor = GetRandomColor();
    clientColors[clientSocket] = clientColor;

    // Send the assigned color to the client
    send(clientSocket, clientColor.c_str(), clientColor.length(), 0);

    char buffer[4096];

    while (true) {
        // Receive message from the client
        int bytesRecvd = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRecvd <= 0) {
            cout << "Client Disconnected" << endl;
            // Remove the disconnected client from the vector
            auto it = std::find(clients.begin(), clients.end(), clientSocket);
            if (it != clients.end()) {
                clients.erase(it);
            }
            break;
        }

        // Parse received message and display with client color
        string clientMsg(buffer, bytesRecvd);
        cout << clientColors.at(clientSocket) << "Client: " << clientMsg << ANSI_COLOR_RESET << endl;

        // Send message to all clients with color information
        for (auto& [socket, color] : clientColors) {
            if (socket != clientSocket) {
                string msgWithColor = color + "Client: " + clientMsg + ANSI_COLOR_RESET;
                send(socket, msgWithColor.c_str(), msgWithColor.length(), 0);
            }
        }
    }

    // Close the socket after disconnection
    closesocket(clientSocket);
}

// Main function
int main() {
    // Initialize Winsock
    if (!Initialize()) {
        cout << "Winsock Initialization Failed!" << endl;
        return 0;
    }

    // Create a socket for listening
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        cout << "Socket Creation Failed!" << endl;
        return 0;
    }

    // Define server address and port
    int port = 12345;
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(port);

    // Bind the server socket to the address
    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "Bind Failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 0;
    }

    // Start listening for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "Listen Failed" << endl;
        closesocket(listenSocket);
        WSACleanup();
        return 0;
    }

    // Print server status
    cout << "Server has started listening on port: " << port << endl;

    // Vector to store client sockets
    vector<SOCKET> clients;
    // Map to store client sockets and assigned colors
    map<SOCKET, string> clientColors;

    // Accept incoming connections and interact with clients
    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            cout << "Invalid client Socket" << endl;
        } else {
            // Add the client socket to the vector
            clients.push_back(clientSocket);

            // Choose a random color for the client
            string clientColor = GetRandomColor();
            // Store the client socket and assigned color in the map
            clientColors[clientSocket] = clientColor;

            // Create a new thread to interact with the client
            thread t1(InteractWithClient, clientSocket, std::ref(clients), std::ref(clientColors));
            t1.detach();
        }
    }

    // Close the listening socket and cleanup Winsock
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
