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

сервер
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










        5555555555555555555555555
            server
            #include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 8888
#define BUF_SIZE 1024

vector<SOCKET> clients;
vector<string> names;
mutex mtx;

void SendAll(const string& msg, SOCKET skip = INVALID_SOCKET) {
    lock_guard<mutex> lock(mtx);
    for (auto client : clients) {
        if (client != skip) send(client, msg.c_str(), (int)msg.size(), 0);
    }
}

int FindUser(const string& name) {
    for (size_t i = 0; i < names.size(); i++)
        if (names[i] == name) return (int)i;
    return -1;
}

void ClientThread(SOCKET sock) {
    char buf[BUF_SIZE];
    int bytes;

    // Получаем ник
    bytes = recv(sock, buf, BUF_SIZE - 1, 0);
    if (bytes <= 0) { closesocket(sock); return; }
    buf[bytes] = '\0';
    string nickname = buf;

    {
        lock_guard<mutex> lock(mtx);
        clients.push_back(sock);
        names.push_back(nickname);
    }

    SendAll(nickname + " вошёл в чат", sock);

    while (true) {
        bytes = recv(sock, buf, BUF_SIZE - 1, 0);
        if (bytes <= 0) break;
        buf[bytes] = '\0';
        string msg = buf;

        if (msg == "/quit") break;

        if (msg.find("/private ") == 0) {
            string rest = msg.substr(9);
            size_t pos = rest.find(' ');
            if (pos != string::npos) {
                string to = rest.substr(0, pos);
                string text = rest.substr(pos + 1);
                int id = FindUser(to);
                if (id != -1) {
                    string out = "Лично от " + nickname + ": " + text;
                    send(clients[id], out.c_str(), (int)out.size(), 0);
                }
            }
        }
        else {
            SendAll(nickname + ": " + msg, sock);
        }
    }

    {
        lock_guard<mutex> lock(mtx);
        for (size_t i = 0; i < clients.size(); i++) {
            if (clients[i] == sock) {
                SendAll(names[i] + " вышел из чата");
                closesocket(clients[i]);
                clients.erase(clients.begin() + i);
                names.erase(names.begin() + i);
                break;
            }
        }
    }
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { cout << "WSAStartup error\n"; return 1; }

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET) { cout << "Socket error\n"; return 1; }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) { cout << "Bind error\n"; return 1; }
    if (listen(server, SOMAXCONN) == SOCKET_ERROR) { cout << "Listen error\n"; return 1; }

    cout << "Server started on port " << PORT << endl;

    while (true) {
        SOCKET client = accept(server, NULL, NULL);
        if (client != INVALID_SOCKET) {
            thread(ClientThread, client).detach();
        }
    }

    closesocket(server);
    WSACleanup();
    return 0;
}
        cout << "Соединение закрыто\n\n";
    }

    closesocket(listener);
    WSACleanup();
    return 0;
}

client
#define _WINSOCK_DEPRECATED_NO_WARNINGS  // Чтобы inet_addr не ругался
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 8888
#define IP "127.0.0.1"
#define BUF_SIZE 1024

SOCKET sock;
bool running = true;

// Поток для приёма сообщений
void ReceiveThread() {
    char buf[BUF_SIZE];
    while (running) {
        int bytes = recv(sock, buf, BUF_SIZE - 1, 0);
        if (bytes > 0) {
            buf[bytes] = '\0';
            cout << buf << endl;
        }
        else {
            running = false;
        }
    }
}

int main() {
    setlocale(LC_ALL, "rus");

    // Инициализация Winsock
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "Ошибка WSAStartup\n";
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cout << "Ошибка сокета\n";
        return 1;
    }

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(IP); // теперь не ругается

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cout << "Не удалось подключиться к серверу\n";
        return 1;
    }

    string name;
    cout << "Введите ник: ";
    getline(cin, name);

    send(sock, name.c_str(), (int)name.size(), 0);

    thread t(ReceiveThread); // запускаем поток для приёма сообщений

    string msg;
    while (running) {
        getline(cin, msg);
        if (!msg.empty()) send(sock, msg.c_str(), (int)msg.size(), 0);
        if (msg == "/quit") break;
    }

    t.join();
    closesocket(sock);
    WSACleanup();

    return 0;
}

