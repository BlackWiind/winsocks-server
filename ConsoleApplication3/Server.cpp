#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <iostream>
#include <direct.h>
#include <string>

using namespace std;

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "4000"
#define DEFAULT_BUFLEN 512


WSADATA wsaData;
char recvbuf[DEFAULT_BUFLEN];
char current_work_dir[FILENAME_MAX];
const char* sendbuf;
const char* bye = "bye";
const char* pwd = "pwd";
int iResult, iSendResult;
struct addrinfo* result = NULL, * ptr = NULL, hints;
SOCKET ListenSocket = INVALID_SOCKET;
SOCKET ClientSocket = INVALID_SOCKET;

void quit() {
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		cout << "shutdown failed with error: " << WSAGetLastError() << '\n';
		closesocket(ClientSocket);
		WSACleanup();
	}

	closesocket(ClientSocket);
	WSACleanup();

	exit(0);
}

void send_msg() {

	iSendResult = send(ClientSocket, sendbuf, (int)strlen(sendbuf), 0);
	if (iSendResult == SOCKET_ERROR) {
		cout << "send failed with error: " << WSAGetLastError() << '\n';
		closesocket(ClientSocket);
		WSACleanup();
	}

}


int __cdecl main() {
	setlocale(LC_ALL, "Russian");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;


	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		cout << "WSAStartup failed: " << iResult << '\n';
		return 1;
	}

	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		cout << "getaddrinfo failed: " << iResult << '\n';
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		cout << "Error at socked(): " << WSAGetLastError() << '\n';
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		cout << "bind failed with error: " << WSAGetLastError() << '\n';
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "listen failed with error: " << WSAGetLastError() << '\n';
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		cout << "accept failed with error: " << WSAGetLastError() << '\n';
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ListenSocket);
	while (true) {
		do {
			iResult = recv(ClientSocket, recvbuf, sizeof(recvbuf) - 1, 0);
			if (iResult > 0) {
				recvbuf[iResult] = 0;
				string command = recvbuf;
				if (strcmp(command.c_str(), bye) == 0) {
					sendbuf = "Отключение...\n";
					send_msg();
					quit();
				}
				else if (command[0] == 'c' && command[1] == 'd') {
					command.erase(0, 3);
					cout << command;
					_chdir(command.c_str());
					sendbuf = "Текущая директория изменена\n";
					send_msg();
				}
				else if (strcmp(command.c_str(), pwd) == 0) {
					sendbuf = _getcwd(current_work_dir, sizeof(current_work_dir));
					system(command.c_str());
					send_msg();
				}
				else {
					sendbuf = "Неверная команда, проверьте правильность ввода!\n";
					send_msg();
				}

			}
		} while (iResult > 0);

		iResult = shutdown(ClientSocket, SD_SEND);
		cout << "shutdown\n";
		if (iResult == SOCKET_ERROR) {
			cout << "shutdown failed with error: " << WSAGetLastError() << '\n';
			closesocket(ClientSocket);
			WSACleanup();
		}
	}
	return 0;
}

