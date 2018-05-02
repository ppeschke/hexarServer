#include "NetworkServer.h"
#include <iostream>
#include <cstring>
using namespace std;

NetworkServer::NetworkServer(unsigned short cl)
{
	if (cl <= 6)
		clients = cl;
	else
	{
		cout << "Cannot have more than 6 clients! Aborting" << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	ClientAddresses = new ClientAddress[clients];
	if (ClientAddresses == nullptr)
	{
		cout << "Allocating memory for clients failed... aborting." << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
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

	for(int i = 0; i < clients; ++i)
		ZeroMemory(&ClientAddresses[i].address, sizeof(sockaddr_in));
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
	if (ClientAddresses != nullptr)
		delete[] ClientAddresses;
	serverfile.close();
	running = false;
	WaitForSingleObject(ListenThreadHandle, INFINITE);	//wait for listen thread to quit
	WSACleanup();
}

//this function is for the listen thread
void NetworkServer::Listen()
{
	unsigned int size;
	while(running)
	{
		size = recvfrom(Socket, Buffer, 256, 0, (sockaddr*)&IncomingAddress, &AddressLength);
		if (size == -1)
			serverfile << "Error: " << WSAGetLastError() << endl;
		if(size > 0)
		{
			Buffer[255] = '\0';	//security! always end packet with this
			if(IncomingAddress.sin_addr.s_addr == ZeroAddress.sin_addr.s_addr)
				continue;
			if(Buffer[0] == 1)	//knock packet
			{
				clientLocker.lock();
				for(int i = 0; i < clients; ++i)	//search for an empty address
				{
					if(!ClientAddresses[i].address.sin_family)
					{
						ClientAddresses[i].address = IncomingAddress;
						strcpy_s(Buffer, "_join ");
						Buffer[6] = i + 49;	//get the ascii character for the number of the client (+1 for non-zero-indexed player numbers in main thread)
						Buffer[7] = '\0';
						serverfile << "Adding to queue: " << Buffer << endl;
						locker.lock();
						messages.insert(messages.end(), Message(Buffer, i));
						locker.unlock();
						break;
					}
				}
				clientLocker.unlock();
			}
			else if(Buffer[0] == 0)	//leave packet
			{
				bool found = false;
				clientLocker.lock();
				for(int i = 0; i < clients; ++i)
				{
					if(ClientAddresses[i].address.sin_addr.s_addr == IncomingAddress.sin_addr.s_addr)
					{
						strcpy_s(Buffer, "_leave ");
						Buffer[7] = i + 49;	//get the ascii character for the number of the client
						Buffer[8] = '\0';
						serverfile << "Adding to queue: " << Buffer << endl;
						locker.lock();
						messages.insert(messages.end(), Message(Buffer, i));
						locker.unlock();
						break;
					}
					else if(ClientAddresses[i].address.sin_family)
						found = true;
				}
				clientLocker.unlock();
				if(!found)
				{
					serverfile << "No clients left... quitting server..." << endl;
					running = false;
				}
			}
			else
			{
				unsigned int client = 42;	//impossible client number
				for(int i = 0; i < clients; ++i)
				{
					if(ClientAddresses[i].address.sin_addr.s_addr == IncomingAddress.sin_addr.s_addr)
					{
						client = i;
						break;
					}
				}
				if(client != 42)
				{
					//add to queue
					serverfile << "Adding to queue: " << Buffer << endl;
					locker.lock();
					messages.insert(messages.end(), Message(Buffer, client));
					locker.unlock();
				}
			}
		}
	}
}

//main thread
void NetworkServer::Broadcast(const char* message)
{
	serverfile << "Broadcasting: " << message << endl;
	for(int i = 0; i < clients; ++i)
	{
		if(ClientAddresses[i].address.sin_family)
			Send(message, i);
	}
}

//main thread
void NetworkServer::Send(const char* message, unsigned int client)
{
	serverfile << "Sending to client " << client << ": " << message << endl;
	strcpy_s(Buffer, message);
	sendto(Socket, Buffer, 256, 0, (sockaddr*)&ClientAddresses[client].address, sizeof(sockaddr));
}

DWORD WINAPI ThreadFunction(LPVOID Whatever)
{
	((NetworkServer*)Whatever)->Listen();
	return 0;
}

unsigned int NetworkServer::ClientCount()
{
	unsigned int count = 0;
	clientLocker.lock();
	for(int i = 0; i < clients; ++i)
	{
		if(ClientAddresses[i].active)
			++count;
	}
	clientLocker.unlock();
	return count;
}

void NetworkServer::ActivateAddress(unsigned int clientNum)
{
	clientLocker.lock();
	ClientAddresses[clientNum].active = true;
	clientLocker.unlock();
}

//this function will run on the main thread
void NetworkServer::DeactivateAddress(unsigned int clientNum)
{
	clientLocker.lock();
	ClientAddresses[clientNum].active = false;
	ZeroMemory(&ClientAddresses[clientNum].address, sizeof(sockaddr_in));
	clientLocker.unlock();
}
