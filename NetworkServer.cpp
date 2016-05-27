#include "NetworkServer.h"
#include <iostream>
using namespace std;

NetworkServer::NetworkServer()
{
	running = true;
	port = 17000;
	AddressLength = sizeof(IncomingAddress);
	WSAStartup(MAKEWORD(2, 2), &Winsock);
	if(LOBYTE(Winsock.wVersion) != 2 || HIBYTE(Winsock.wVersion) != 2)
	{
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	for(int i = 0; i < CLIENTS; ++i)
		ZeroMemory(&ClientAddresses[i], sizeof(sockaddr_in));

	ZeroMemory(&ServerAddress, AddressLength);
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = port;
	bind(Socket, (sockaddr*)&ServerAddress, AddressLength);

	ListenThreadHandle = CreateThread(NULL, 0, ThreadFunction, this, 0, NULL);
}


NetworkServer::~NetworkServer()
{
	running = false;
	WaitForSingleObject(ListenThreadHandle, INFINITE);	//wait for listen thread to quit
	WSACleanup();
}

void NetworkServer::Listen()
{
	while(running)
	{
		if(recvfrom(Socket, Buffer, 256, 0, (sockaddr*)&IncomingAddress, &AddressLength))
		{
			Buffer[255] = '\0';	//security! always end packet with this

			if(Buffer[0] == 1)	//knock packet
			{
				for(int i = 0; i < CLIENTS; ++i)	//search for an empty address
				{
					if(!ClientAddresses[i].sin_family)
					{
						ClientAddresses[i] = IncomingAddress;
						//cout << "Server Broadcasting: Client has connected. Welcome!" << endl;
						Broadcast("Client has connected. Welcome!");
						break;
					}
				}
			}
			else if(Buffer[0] == 0)	//leave packet
			{
				bool found = false;
				for(int i = 0; i < CLIENTS; ++i)
				{
					if(ClientAddresses[i].sin_addr.s_addr == IncomingAddress.sin_addr.s_addr)
					{
						Buffer[0] = 0;
						Send(Buffer, i);
						ZeroMemory(&ClientAddresses[i], sizeof(sockaddr_in));
						//cout << "Client " << i << " has disconnected." << endl;
						Broadcast("Client has disconnected.");
					}
					else if(ClientAddresses[i].sin_family)
						found = true;
				}
				if(!found)
				{
					//cout << "No clients left... quitting server..." << endl;
					running = false;
				}
			}
			else
			{
				unsigned int client = 42;	//secret number
				for(int i = 0; i < CLIENTS; ++i)
				{
					if(ClientAddresses[i].sin_addr.s_addr == IncomingAddress.sin_addr.s_addr)
					{
						client = i;
						break;
					}
				}
				if(client != 42)
				{
					//add to queue
					locker.lock();
					messages.insert(messages.end(), Message(Buffer, client));
					locker.unlock();
				}
			}
		}
	}
}

void NetworkServer::Broadcast(const char* message)
{
	for(int i = 0; i < CLIENTS; ++i)
	{
		if(ClientAddresses[i].sin_family)
			Send(message, i);
	}
}

void NetworkServer::Send(const char* message, unsigned int client)
{
	sendto(Socket, message, 256, 0, (sockaddr*)&ClientAddresses[client], sizeof(sockaddr));
}

DWORD WINAPI ThreadFunction(LPVOID Whatever)
{
	((NetworkServer*)Whatever)->Listen();
	return 0;
}

unsigned int NetworkServer::ClientCount()
{
	unsigned int count = 0;
	for(int i = 0; i < CLIENTS; ++i)
	{
		if(ClientAddresses[i].sin_family)
			++count;
	}
	return count;
}