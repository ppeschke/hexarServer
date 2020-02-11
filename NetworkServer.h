#ifndef NETWORK_SERVER
#define NETWORK_SERVER


#include <list>
#include <mutex>
#include <fstream>
using namespace std;
#include "Message.h"
#include "ClientAddress.h"

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
	unsigned short MaxClients() { return maxClients; }
	unsigned short ActiveClients() { return activeClients; }

	ofstream serverfile;
	mutex locker;
	mutex clientLocker;
	list<Message> messages;
	unsigned int listenPort;
	WSAData Winsock;
	SOCKET ListenSocket;
	sockaddr_in ServerAddress;
	SOCKET ClientSockets[6];
	char Buffer[256];
	int AddressLength;
	volatile bool running;
	HANDLE ListenThreadHandle;

private:
	unsigned short maxClients;
	unsigned short activeClients;
};

#endif