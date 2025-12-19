#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8000
#define BUFFER_SIZE 1024

using namespace std;

void processClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int received = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

    if (received <= 0) {
        closesocket(clientSocket);
        return;
    }

    buffer[received] = '\0';
    cout << "[REQUEST]\n" << buffer << endl;

    string body =
        "<html><body>"
        "<h1>rgjiotjg HTTP Server</h1>"
        "<p>HTTP interaction via TCP sockets</p>"
        "<pre>" + string(buffer) + "</pre>"
        "</body></html>";

    string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: " + to_string(body.size()) + "\r\n"
        "Connection: close\r\n\r\n" +
        body;

    send(clientSocket, response.c_str(), response.length(), 0);
    shutdown(clientSocket, SD_SEND);
    closesocket(clientSocket);
}

int main() {
    setlocale(LC_ALL, "rus");

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&addr, sizeof(addr));
    listen(serverSocket, SOMAXCONN);

    cout << "[INFO] HTTP-сервер запущен на порту " << PORT << endl;

    while (true) {
        SOCKET client = accept(serverSocket, nullptr, nullptr);
        if (client != INVALID_SOCKET) {
            cout << "[INFO] Клиент подключился\n";
            processClient(client);
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}   

client

#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define BUFFER_SIZE 4096

using namespace std;

bool sendHttpGetRequest(const string& host, const string& port, const string& request) {
    addrinfo hints{};
    addrinfo* addressInfo = nullptr;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int result = getaddrinfo(host.c_str(), port.c_str(), &hints, &addressInfo);
    if (result != 0) {
        cout << "[ERROR] getaddrinfo failed: " << result << endl;
        return false;
    }

    SOCKET clientSocket = socket(
        addressInfo->ai_family,
        addressInfo->ai_socktype,
        addressInfo->ai_protocol
    );

    if (clientSocket == INVALID_SOCKET) {
        cout << "[ERROR] Socket creation failed\n";
        freeaddrinfo(addressInfo);
        return false;
    }

    cout << "[INFO] Connecting to " << host << ":" << port << "...\n";

    if (connect(clientSocket, addressInfo->ai_addr,
        (int)addressInfo->ai_addrlen) == SOCKET_ERROR) {

        cout << "[ERROR] Connection failed\n";
        closesocket(clientSocket);
        freeaddrinfo(addressInfo);
        return false;
    }

    freeaddrinfo(addressInfo);
    cout << "[INFO] Connected successfully\n";

    if (send(clientSocket, request.c_str(),
        (int)request.length(), 0) == SOCKET_ERROR) {

        cout << "[ERROR] Failed to send request\n";
        closesocket(clientSocket);
        return false;
    }

    cout << "[INFO] Request sent, waiting for response...\n\n";

    char buffer[BUFFER_SIZE];
    int receivedBytes;
    int totalBytes = 0;

    while ((receivedBytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[receivedBytes] = '\0';
        cout << buffer;
        totalBytes += receivedBytes;
    }

    cout << "\n\n[INFO] Total bytes received: " << totalBytes << endl;

    closesocket(clientSocket);
    return true;
}

int main() {
    setlocale(LC_ALL, "rus");

    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "[ERROR] WSAStartup failed\n";
        return -1;
    }

    cout << "\tHTTP Client (C++)\n";
    cout << "==============================\n";

    while (true) {
        int choice;
        cout << "\nSelect connection type:\n";
        cout << "1 - Local HTTP server\n";
        cout << "2 - External web server (example.com)\n";
        cout << "0 - Exit\n";
        cout << "Your choice: ";
        cin >> choice;

        if (choice == 0) break;

        if (choice == 1) {
            string request =
                "GET / HTTP/1.1\r\n"
                "Host: localhost:8000\r\n"
                "User-Agent: Simple-Cpp-Client\r\n"
                "Connection: close\r\n\r\n";

            sendHttpGetRequest("localhost", "8000", request);
        }
        else if (choice == 2) {
            string request =
                "GET / HTTP/1.1\r\n"
                "Host: www.example.com\r\n"
                "User-Agent: Simple-Cpp-Client\r\n"
                "Connection: close\r\n\r\n";

            sendHttpGetRequest("www.example.com", "80", request);
        }
        else {
            cout << "[WARNING] Invalid option\n";
        }

        cout << "\n------------------------------\n";
    }

    WSACleanup();
    return 0;
}
