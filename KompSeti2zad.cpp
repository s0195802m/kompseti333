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
}.  3 задание
Сервер:
#include <iostream>
#include <sstream>
#include <string>

#define _WIN32_WINNT 0x501
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#define BUF_SIZE 1024

using namespace std;

int main() {
    setlocale(LC_ALL, "rus");
    cout << "\t HTTP-сервер\n";
    for (int i = 0; i < 30; i++) cout << "=";
    cout << "\n";

    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws)) {
        cerr << "Ошибка WSAStartup! \n" << WSAGetLastError();
        return -1;
    }

    // Создаем сокет
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET) {
        cerr << "Ошибка socket! \n" << WSAGetLastError();
        WSACleanup();
        return -1;
    }

    // Устанавливаем опцию переиспользования адреса
    int optval = 1;
    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval)) == SOCKET_ERROR) {
        cerr << "Ошибка setsockopt! \n" << WSAGetLastError();
        closesocket(listener);
        WSACleanup();
        return -1;
    }

    // Настраиваем адрес сервера
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Слушаем все интерфейсы
    serverAddr.sin_port = htons(8000);

    // Привязываем сокет к адресу
    if (bind(listener, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Ошибка bind! \n" << WSAGetLastError();
        closesocket(listener);
        WSACleanup();
        return -1;
    }

    // Начинаем слушать порт
    if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Ошибка listen! \n" << WSAGetLastError();
        closesocket(listener);
        WSACleanup();
        return -1;
    }

    cout << "Сервер запущен на порту 8000\n";
    cout << "Ожидание подключений...\n\n";

    char buf[BUF_SIZE];

    while (true) {
        SOCKET sClient = accept(listener, NULL, NULL);
        if (sClient == INVALID_SOCKET) {
            cerr << "Ошибка accept! " << WSAGetLastError() << "\n";
            continue;
        }

        cout << "Присоединился новый клиент!\n";

        // Получаем запрос
        int len = recv(sClient, buf, BUF_SIZE - 1, 0);

        if (len == SOCKET_ERROR) {
            cerr << "Ошибка recv! " << WSAGetLastError() << "\n";
            closesocket(sClient);
            continue;
        }

        if (len == 0) {
            cout << "Соединение закрыто клиентом\n";
            closesocket(sClient);
            continue;
        }

        buf[len] = '\0';
        cout << "Получен запрос от клиента\n";

        // Формируем ответ
        stringstream responseBody;
        responseBody << "<html><body>"
            << "<h1> C++ HTTP Server</h1>"
            << "<p>Work with sockets is demonsrated successfully </p>"
            << "<pre>Request: " << buf << "</pre>"
            << "</body></html>";

        stringstream response;
        response << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            << "Content-Length: " << responseBody.str().length() << "\r\n"
            << "Connection: close\r\n"
            << "\r\n"
            << responseBody.str();

        // Отправляем ответ
        int sendResult = send(sClient, response.str().c_str(), response.str().length(), 0);
        if (sendResult == SOCKET_ERROR) {
            cerr << "Ошибка send! " << WSAGetLastError() << "\n";
        }
        else {
            cout << "Ответ отправлен (" << sendResult << " байт)\n";
        }

        // Корректно закрываем соединение
        shutdown(sClient, SD_SEND); // Останавливаем отправку
        closesocket(sClient);
        cout << "Соединение закрыто\n\n";
    }

    closesocket(listener);
    WSACleanup();
    return  
3 задание
Клиент:
#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#define BUF_SIZE 4096

using namespace std;

void HTTP_Connection(string host, string request) {
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
        cout << "Ошибка WSAStartup! \n";
        return;
    }

    // Используем getaddrinfo для современного разрешения адресов
    struct addrinfo hints, * result;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    string port = (host == "localhost") ? "8000" : "80";

    int iResult = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
    if (iResult != 0) {
        cout << "Ошибка getaddrinfo! " << iResult << "\n";
        WSACleanup();
        return;
    }

    SOCKET s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (s == INVALID_SOCKET) {
        cout << "Ошибка socket! " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        WSACleanup();
        return;
    }

    // Устанавливаем таймаут
    int timeout = 3000; // 3 секунды
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    cout << "Подключаемся к " << host << ":" << port << "...\n";

    if (connect(s, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        cout << "Ошибка connect! " << WSAGetLastError() << "\n";
        freeaddrinfo(result);
        closesocket(s);
        WSACleanup();
        return;
    }

    freeaddrinfo(result);
    cout << "Подключение установлено. Отправляем запрос...\n";

    if (send(s, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        cout << "Ошибка send! " << WSAGetLastError() << "\n";
        closesocket(s);
        WSACleanup();
        return;
    }

    cout << "Запрос отправлен. Получаем ответ...\n\n";

    // Получаем ответ
    char buf[BUF_SIZE + 1];
    int total_bytes = 0;
    int len;

    while ((len = recv(s, buf, BUF_SIZE, 0)) > 0) {
        buf[len] = '\0';
        cout << buf;
        total_bytes += len;
    }

    if (len == 0) {
        cout << "\nСервер закрыл соединение\n";
    }
    else if (len == SOCKET_ERROR) {
        int error = WSAGetLastError();
        if (error == WSAETIMEDOUT) {
            cout << "\nТаймаут при получении данных\n";
        }
        else {
            cout << "Ошибка recv! " << error << "\n";
        }
    }

    cout << "\nВсего получено байт: " << total_bytes << "\n";

    closesocket(s);
    WSACleanup();
}

int main() {
    setlocale(LC_ALL, "rus");

    cout << "\t Веб-клиент\n";
    for (int i = 0; i < 30; i++) cout << "=";
    cout << endl;

    // Инициализируем Winsock один раз
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) != 0) {
        cerr << "Ошибка WSAStartup! \n" << WSAGetLastError();
        return -1;
    }

    while (true) {
        int type;
        cout << "Введите тип подключения (1 - локальный сервер, 2 - example.com, 0 - выход): ";
        cin >> type;

        if (type == 0) break;

        if (type == 1) {
            HTTP_Connection("localhost",
                "GET / HTTP/1.1\r\n"
                "Host: localhost:8000\r\n"
                "User-Agent: C++-Client\r\n"
                "Connection: close\r\n"
                "\r\n");
        }
        else if (type == 2) {
            HTTP_Connection("www.example.com",
                "GET / HTTP/1.1\r\n"
                "Host: www.example.com\r\n"
                "User-Agent: C++-Client\r\n"
                "Connection: close\r\n"
                "\r\n");
        }
        else {
            cout << "Неверный выбор!\n";
        }

        cout << "\nГотово!\n";
        for (int i = 0; i < 30; i++) cout << "-";
        cout << endl;
    }

    WSACleanup();
    return 0;
}




