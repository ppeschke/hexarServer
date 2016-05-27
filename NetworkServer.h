#ifndef NETWORK_SERVER
#define NETWORK_SERVER

#define _WINSOCKAPI_
#include <WinSock2.h>
#include <list>
#include <mutex>
using namespace std;
#include "Message.h"

#pragma comment(lib, "Ws2_32.lib")

DWORD WINAPI ThreadFunction(LPVOID Whatever);

#define CLIENTS 2

class NetworkServer
{
public:
	NetworkServer();
	~NetworkServer();

	void Listen();
	void Send(const char* message, unsigned int clientNum);
	void Broadcast(const char* message);
	unsigned int ClientCount();

	mutex locker;
	list<Message> messages;
	unsigned int port;
	WSAData Winsock;
	SOCKET Socket;
	sockaddr_in ServerAddress;
	sockaddr_in IncomingAddress;
	sockaddr_in ClientAddresses[CLIENTS];
	char Buffer[256];
	int AddressLength;
	volatile bool running;
	HANDLE ListenThreadHandle;
};

#endif