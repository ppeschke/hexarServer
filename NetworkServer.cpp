#include "NetworkServer.h"
#include <iostream>
#include <cstring>
using namespace std;

NetworkServer::NetworkServer()
{
	serverfile.open("NetworkServer.log");
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
	ZeroMemory(&ZeroAddress, sizeof(sockaddr_in));

	ZeroMemory(&ServerAddress, AddressLength);
	ServerAddress.sin_family = AF_INET;
	u_short IpnetShort;
	WSAHtons(Socket, port, &IpnetShort);
	ServerAddress.sin_port =  IpnetShort;
	int result = ::bind(Socket, (sockaddr*)&ServerAddress, AddressLength);

	ListenThreadHandle = CreateThread(NULL, 0, ThreadFunction, this, 0, NULL);
}


NetworkServer::~NetworkServer()
{
	serverfile.close();
	running = false;
	WaitForSingleObject(ListenThreadHandle, INFINITE);	//wait for listen thread to quit
	WSACleanup();
}

void NetworkServer::Listen()
{
	unsigned int size;
	while(running)
	{
		size = recvfrom(Socket, Buffer, 256, 0, (sockaddr*)&IncomingAddress, &AddressLength);
		if (size == -1)
			cout << "Error: " << WSAGetLastError() << endl;
		if(size > 0)
		{
			Buffer[255] = '\0';	//security! always end packet with this
			if(IncomingAddress.sin_addr.s_addr == ZeroAddress.sin_addr.s_addr)
				continue;
			if(Buffer[0] == 1)	//knock packet
			{
				for(int i = 0; i < CLIENTS; ++i)	//search for an empty address
				{
					if(!ClientAddresses[i].sin_family)
					{
						ClientAddresses[i] = IncomingAddress;
						strcpy_s(Buffer, " (Acknowledge Packet)");
						Buffer[0] = 1;
						cout << "Sending Acknowledgement" << endl;
						Send(Buffer, i);	//acknowledge knock
						cout << "Server Broadcasting: Client has connected. Welcome!" << endl;
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
						Buffer[0] = '\0';
						Send(Buffer, i);
						ZeroMemory(&ClientAddresses[i], sizeof(sockaddr_in));
						cout << "Client " << i << " has disconnected." << endl;
						Broadcast("Client has disconnected.");
						break;
					}
					else if(ClientAddresses[i].sin_family)
						found = true;
				}
				if(!found)
				{
					cout << "No clients left... quitting server..." << endl;
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
					cout << "Adding to queue: " << Buffer << endl;
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
	serverfile << "Broadcasting: " << message << endl;
	for(int i = 0; i < CLIENTS; ++i)
	{
		if(ClientAddresses[i].sin_family)
			Send(message, i);
	}
}

void NetworkServer::Send(const char* message, unsigned int client)
{
	serverfile << "Sending to client " << client << ": " << message << endl;
	strcpy_s(Buffer, message);
	sendto(Socket, Buffer, 256, 0, (sockaddr*)&ClientAddresses[client], sizeof(sockaddr));
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