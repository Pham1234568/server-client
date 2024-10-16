#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct User {
    std::string username;
    std::string password;
    SOCKET socket;
    int check;
};

std::vector<User> users;
std::vector<std::thread> clientThreads;

bool loadUsers() {
    std::ifstream file("users.txt");
    if (!file.is_open()) {
        std::cerr << "Unable to open users.txt file" << std::endl;
        return false;
    }

    User user;
    while (file >> user.username >> user.password) {
        user.socket = INVALID_SOCKET;  // Socket will be assigned after login
        user.check = 0;
        users.push_back(user);
    }

    file.close();
    return true;
}

bool saveUser(const User& user) {
    std::ofstream file("users.txt", std::ios_base::app);
    if (!file.is_open()) {
        std::cerr << "Unable to open users.txt file" << std::endl;
        return false;
    }
    file << user.username << " " << user.password << std::endl;

    file.close();
    return true;
}

void broadcastMessage(const std::string& message, SOCKET senderSocket) {
    for (const User& user : users) {
        if (user.socket != INVALID_SOCKET && user.socket != senderSocket) {
            send(user.socket, message.c_str(), message.size() + 1, 0);
        }
    }
}

void sendMessageToUser(const std::string& message, const std::string& recipientUsername, SOCKET senderSocket) {
    for (const User& user : users) {
        if (user.socket != INVALID_SOCKET && user.socket != senderSocket && user.username == recipientUsername) {
            send(user.socket, message.c_str(), message.size() + 1, 0);
        }
    }
}

// Function to handle a single client
void handleClient(SOCKET clientSocket) {
    char buffer[1024];
    int bytesReceived;
    
    // Receive the username and password from the client for login or registration
    while (1)
    {
        recv(clientSocket, buffer, sizeof(buffer), 0);
        std::string choice(buffer);
        
        if (choice == "1") {
            // Register
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::string username(buffer);
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::string password(buffer);
            User newUser;
            bool ktra=true;
            for (User& user : users) {
                    if (strcmp(username.c_str(),user.username.c_str())==0)
                    {
                        send(clientSocket, "register_fail,username has been used", sizeof("register_fail, username has been used"), 0);
                        ktra=false;
                        break;
                    }
            }
            if(ktra){
            newUser.username = username;
            newUser.password = password;
            newUser.socket = INVALID_SOCKET;
            newUser.check = 0;
            users.push_back(newUser);
            if (saveUser(newUser)) {
                send(clientSocket, "register_success", sizeof("register_success"), 0);
            } }
        } else if (choice == "2") {
            // Login
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::string username = buffer;
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::string password = buffer;
            User b;
            b.username = username;
            b.password = password;
            bool loginSuccess = false;
            for (User& user : users) {
                if (user.username == b.username && user.password == b.password && user.socket == INVALID_SOCKET &&
                    user.check == 0) {
                    user.socket = clientSocket;
                    user.check = 1;
                    loginSuccess = true;
                    break;
                }
            }

            if (loginSuccess) {
                send(clientSocket, "login_success:", sizeof("login_success:"), 0);

                while (1)
                {
                    recv(clientSocket, buffer, sizeof(buffer), 0);
                    std::string choose(buffer);
                    if (choose == "1") {
                        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                        string name = buffer;
                        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
                        string message = buffer;
                        // Unicast message to a specific user
                        sendMessageToUser(message, name, clientSocket);
                    }

                    else if (choose == "2") {
                        recv(clientSocket, buffer, sizeof(buffer), 0);
                        string message = buffer;

                        // Broadcast message to all connected clients
                        broadcastMessage(message, clientSocket);
                    } 
                    else if(choose == "3"){
                        recv(clientSocket, buffer, sizeof(buffer), 0);
                        string message = buffer;
                        string onlineUsers = "Users are online: ";
                        for (User& user : users) {
                            if(user.check == 1){
                                if(strcmp(message.c_str(),user.username.c_str())==0){
                                    continue;
                                }
                                onlineUsers += "[" + user.username + "]" + "     ";
                            }
                        }
                        send(clientSocket,onlineUsers.c_str(),onlineUsers.size()+1,0);   
                    }
                    else if (choose == "4") // logout
                    {
                        recv(clientSocket, buffer, sizeof(buffer), 0);
                        string message = buffer;
                        for (User& user : users) {
                            if (strcmp(message.c_str(),user.username.c_str())==0) {
                               user.check = 0;
                               user.socket = INVALID_SOCKET;
                               send(clientSocket,"You was logged out",sizeof("You was logged out"),0);
                               break;
                            }
                        }
                        
                        break;
                    }      
                }
                
            } else {
                send(clientSocket, "login_fail", sizeof("login_fail"), 0);
            }
        }
    }
    // Clean up and close the client socket
    // closesocket(clientSocket);
    std::cout << "Client disconnected" << endl;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return -1;
    }

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket" << std::endl;
        WSACleanup();
        return -1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    loadUsers();
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Error binding socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Error listening on socket" << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return -1;
    }

    std::cout << "Server is listening for incoming connections..." << std::endl;

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);

        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Error accepting connection" << std::endl;
            closesocket(serverSocket);
            WSACleanup();
            return -1;
        }

        std::cout << "Client connected" << std::endl;

        // Create a new thread to handle the client
        std::thread clientThread(handleClient, clientSocket);
        clientThreads.push_back(std::move(clientThread));
    }

    // Cleanup
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
