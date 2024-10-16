#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <atomic>
#include <fstream>
std::atomic<bool> shouldExit;
#pragma comment(lib, "ws2_32.lib")
using namespace std;

void receiveMessages(SOCKET clientSocket) {
    char buffer[1024];
    int bytesReceived;

    while (!shouldExit) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            // Server disconnected or an error occurred
            std::cout << "Server disconnected" << std::endl;
            break;
        }
        std::cout << buffer << std::endl;
    }
}
void savemessage(string message)
{
    std::ofstream file("message.txt", std::ios_base::app);
    if (!file.is_open()) {
        std::cerr << "Unable to open users.txt file" << std::endl;
    }
    file << message << std::endl;

    file.close();
}
int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return -1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error connecting to server" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Connected to server" << std::endl;
    while (1)
    {
        std::cout << "1. Register\n2. Login\nChoose option: ";
        int choice;
        std::cin >> choice;

        send(clientSocket, std::to_string(choice).c_str(), sizeof(choice), 0);

        if (choice == 1) {
            // Register
            std::string username, password;
            std::cout << "Enter username: ";
            std::cin >> username;
            send(clientSocket, username.c_str(), strlen(username.c_str()) + 1, 0);
            std::cout << "Enter password: ";
            std::cin >> password;
            send(clientSocket, password.c_str(), strlen(password.c_str()) + 1, 0);

            char registerResponse[256];
            recv(clientSocket, registerResponse, sizeof(registerResponse), 0);
            std::cout << "Server response: " << registerResponse << std::endl;
        } else if (choice == 2) {
            // Login
            std::string username, password;
            std::cout << "Enter username: ";
            std::cin.ignore();
            std::getline(std::cin, username);
            send(clientSocket, username.c_str(), strlen(username.c_str()) + 1, 0);
            std::cout << "Enter password: ";
            std::getline(std::cin, password);

            send(clientSocket, password.c_str(), strlen(password.c_str()) + 1, 0);

            char loginResponse[256];
            recv(clientSocket, loginResponse, sizeof(loginResponse), 0);
            std::cout << "Server response: " << loginResponse << std::endl;

            std::string recipientUsername;
            string choose;

            // After sending the recipient's username, send the message to the server
            if (strcmp(loginResponse, "login_success:") == 0) {
                shouldExit = false;
                std::thread receiveThread(receiveMessages, clientSocket);
                // Send messages to the server
                while (true) {
                    std:cout<<"|---------------------------------------------------------------------------|\n";
                    std::cout << "|Unicast enter 1--Broadcast enter 2--User are online enter 3--LogOut enter 4|\n";
                    std::cout <<"|---------------------------------------------------------------------------|" << endl;
                    cin >> choose;
                    send(clientSocket, choose.c_str(), strlen(choose.c_str()) + 1, 0);
                    std::string message;
                    std::cin.ignore();
                    if (choose == "1") {
                        std::cout << "Enter the username of the recipient:";
                        std::getline(cin, recipientUsername);
                        send(clientSocket, recipientUsername.c_str(), strlen(recipientUsername.c_str()) + 1, 0);
                        std::cout << "Enter your message: ";
                        std::getline(cin, message);
                        message = "\nUser ["+ username + "] send message to YOU: " + message+"\n";
                        savemessage(message);
                        send(clientSocket, message.c_str(), strlen(message.c_str()) + 1, 0);
                    } else if (choose == "2") {
                        std::cout << "Enter your message: ";
                        std::getline(cin, message);
                        message = "\nUser [" + username + "] send message to ALL: " + message + "\n";
                        savemessage(message);
                        send(clientSocket, message.c_str(), strlen(message.c_str()) + 1, 0);
                    }
                    else if(choose == "3"){
                        message = username;
                        send(clientSocket, message.c_str(), strlen(message.c_str()) + 1, 0);
                    }
                    else if (choose == "4")
                    {
                        message = username;
                        send(clientSocket, message.c_str(), strlen(message.c_str()) + 1, 0);
                        break;
                    }
                }
                shouldExit = true;
                receiveThread.join();
            }
        }
    }
    

    // Cleanup
    closesocket(clientSocket);
    WSACleanup();
    
    return 0;
}
