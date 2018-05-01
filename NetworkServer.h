#ifndef NETWORK_SERVER
#define NETWORK_SERVER

#define _WINSOCKAPI_
#include <WinSock2.h>
#include <list>
#include <mutex>
#include <fstream>
using namespace std;
#include "Message.h"

#pragma comment(lib, "Ws2_32.lib")

DWORD WINAPI ThreadFunction(LPVOID Whatever);

class NetworkServer
{
public:
	NetworkServer(unsigned short clients);
	~NetworkServer();

	void Listen();
	void Send(const char* message, unsigned int clientNum);
	void Broadcast(const char* message);
	unsigned int ClientCount();
	unsigned short MaxClients() { return clients; }

	ofstream serverfile;
	mutex locker;
	list<Message> messages;
	unsigned int port;
	WSAData Winsock;
	SOCKET Socket;
	sockaddr_in ServerAddress;
	sockaddr_in IncomingAddress;
	sockaddr_in ZeroAddress;
	sockaddr_in* ClientAddresses;
	char Buffer[256];
	int AddressLength;
	volatile bool running;
	HANDLE ListenThreadHandle;

private:
	unsigned short clients;
};

#endif