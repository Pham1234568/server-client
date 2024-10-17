Socket Programming Server-Client Application

This project is a server-client application developed using C++ and socket programming. It allows multiple clients to connect to a server, register and log in, and communicate through direct (unicast) or broadcast messaging. The server manages the connected clients, tracks online users, and supports logging out.

Features
User Registration: New users can register with a unique username and password.
User Login: Existing users can log in to access the messaging features.
Unicast Messaging: Send direct messages to specific online users.
Broadcast Messaging: Send messages to all connected users.
Online User Display: View a list of currently online users.
Logout Functionality: Users can log out, freeing up their connections for future logins.
Message Storage: Client-side message storage for chat history.
Technology Stack
Programming Language: C++

Library: WinSock2 (Windows Sockets API) for socket programming
Threading: C++ <thread> library for handling multiple clients
Requirements
Windows OS
Visual Studio or any C++ IDE that supports Windows development
Git (for version control)
Installation and Setup

Clone the Repository:
git clone (https://github.com/Pham1234568/server-client.git).
Open the Project in Visual Studio (or your preferred C++ IDE).
Build the Solution: Ensure you have linked the ws2_32.lib library properly.
Run the Server:
Build and run the server.cpp file first to start the server.
Run the Client:
Open a new terminal or command prompt and run the client.cpp file to connect to the server.
Usage:
Register: Enter your username and password to register.
Login: Log in with your credentials.
Send Messages:
Unicast: Send a direct message to another online user.
Broadcast: Send a message to all connected clients.
View Online Users: See a list of currently connected and online users.
Logout: Disconnect from the server.

File Structure
server.cpp: Contains the server-side logic for handling clients and managing messages.
client.cpp: Contains the client-side code for connecting to the server and sending/receiving messages.
users.txt: A text file storing registered usernames and passwords.
message.txt: A text file for storing chat messages locally on the client side.

Contact
For any questions or suggestions, feel free to reach out:
Email: tungpham02363758@gmail.com
