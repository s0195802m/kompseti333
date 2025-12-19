#include <winsock2.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct Employee {
    char name[50];
    int projects;
    int overtime;
    int efficiency;
    int initiatives;
};

int main() {
    setlocale(LC_ALL, "Russian");

    WSADATA w;
    WSAStartup(MAKEWORD(2, 2), &w);

    SOCKET serv = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(1234);

    bind(serv, (sockaddr*)&addr, sizeof(addr));
    listen(serv, 5);

    cout << "Сервер запущен. Ожидание клиента...\n";

    while (true) {
        sockaddr_in cli;
        int cl = sizeof(cli);

        SOCKET client = accept(serv, (sockaddr*)&cli, &cl);
        cout << "Клиент подключился.\n";

        while (true) {
            Employee E;
            int r = recv(client, (char*)&E, sizeof(E), 0);

            if (r <= 0) {
                cout << "Клиент отключился.\n";
                break;
            }

            if (string(E.name) == "exit") {
                cout << "Клиент завершил работу.\n";
                break;
            }

            cout << "\nПолучены данные:\n";
            cout << "Имя: " << E.name << endl;
            cout << "Проекты: " << E.projects << endl;
            cout << "Сверхурочные: " << E.overtime << endl;
            cout << "Эффективность: " << E.efficiency << endl;
            cout << "Инициативы: " << E.initiatives << endl;

            int score = E.projects * 2 + E.overtime + E.efficiency * 3 + E.initiatives * 2;

            string bonus;
            if (score < 10) bonus = "NO_BONUS";
            else if (score < 20) bonus = "STANDARD_BONUS";
            else if (score < 30) bonus = "MEDIUM_BONUS";
            else bonus = "HIGH_BONUS";

            send(client, bonus.c_str(), bonus.size() + 1, 0);
        }

        closesocket(client);
    }

    closesocket(serv);
    WSACleanup();
    return 0;
}