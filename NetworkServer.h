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
	unsigned int ClientCount();
	unsigned short MaxClients() { return clients; }
	void ActivateAddress(unsigned int clientNum);
	void DeactivateAddress(unsigned int clientNum);

	ofstream serverfile;
	mutex locker;
	mutex clientLocker;
	list<Message> messages;
	unsigned int port;
	WSAData Winsock;
	SOCKET Socket;
	sockaddr_in ServerAddress;
	sockaddr_in IncomingAddress;
	sockaddr_in ZeroAddress;
	ClientAddress* ClientAddresses;
	char Buffer[256];
	int AddressLength;
	volatile bool running;
	HANDLE ListenThreadHandle;

private:
	unsigned short clients;
};

#endif