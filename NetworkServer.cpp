#include "NetworkServer.h"
#include <iostream>
#include <cstring>
#include <ws2tcpip.h>
using namespace std;

NetworkServer::NetworkServer(unsigned short cl)
{
	int opt = 1;	//true for reusable address (listensocket)

	running = false;
	if (cl <= 6)
	{
		maxClients = cl;
		activeClients = 0;
	}
	else
	{
		cout << "Cannot have more than 6 clients! Aborting" << endl;
		system("pause");
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < 6; ++i)
	{
		ClientSockets[i] = 0;
	}
	serverfile.open("NetworkServer.log");
	listenPort = 17000;
	WSAStartup(MAKEWORD(2, 2), &Winsock);
	if(LOBYTE(Winsock.wVersion) != 2 || HIBYTE(Winsock.wVersion) != 2)
	{
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (ListenSocket == 0)
	{
		serverfile << "Failed to create listen socket." << endl;
		WSACleanup();
		return;
	}

	if (setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0)
	{
		serverfile << "Failed to set socket options." << endl;
		closesocket(ListenSocket);
		WSACleanup();

	}

	ZeroMemory(&ServerAddress, sizeof(ServerAddress));
	ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_addr.s_addr = INADDR_ANY;
	ServerAddress.sin_port = htons(listenPort);

	if (::bind(ListenSocket, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0)
	{
		serverfile << "Failed to bind." << endl;
		closesocket(ListenSocket);
		WSACleanup();
		return;
	}

	running = true;

	ListenThreadHandle = CreateThread(NULL, 0, ThreadFunction, this, 0, NULL);
}


NetworkServer::~NetworkServer()
{
	serverfile.close();
	running = false;
	WaitForSingleObject(ListenThreadHandle, INFINITE);	//wait for listen thread to quit
	WSACleanup();
}

//this function is for the listen thread
void NetworkServer::Listen()
{
	bool				listeningForNew = true;
	ofstream			listenerLog("ListenThread.log");
	int					activity;
	fd_set				readFDs;	//fd's for select
	struct timeval		timeout;
	SOCKET				new_socket;
	struct sockaddr_in	address;
	int					addressLen = sizeof(address);
	int					valRead;

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	while (running)
	{
		FD_ZERO(&readFDs);

		if (listeningForNew)
		{
			if (listen(ListenSocket, maxClients - activeClients) < 0)
			{
				listenerLog << "Failure listening." << endl;
				running = false;
				break;
			}
			//add the listen socket to the select list
			FD_SET(ListenSocket, &readFDs);
		}

		//add any active clients to the select list
		for (int i = 0; i < maxClients; ++i)
		{
			int sd = ClientSockets[i];
			if (sd > 0)
			{
				FD_SET(sd, &readFDs);
			}
		}

		activity = select(0, &readFDs, NULL, NULL, &timeout);
		if (activity == 0)
		{
			continue;	//timeout
		}
		else if (activity == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			listenerLog << "Select failed with code " << error << endl;
			running = false;
			break;
		}

		if (listeningForNew && FD_ISSET(ListenSocket, &readFDs))
		{
			//incoming connection
			listenerLog << "New Connection! Huzzah!" << endl;
			if (INVALID_SOCKET == (new_socket = accept(ListenSocket, (struct sockaddr *)&address, &addressLen)))
			{
				int error = WSAGetLastError();
				if (error == WSAEFAULT)
				{
					listenerLog << "Accepting new connection failed with code " << error << endl;
				}
				running = false;
				break;
			}
			else
			{
				for (int i = 0; i < maxClients; ++i)
				{
					if (ClientSockets[i] == 0)
					{
						ClientSockets[i] = new_socket;
						new_socket = 0;
						++activeClients;
						break;
					}
				}
			}

			for (int i = 0; i < maxClients; ++i)
			{
				if (ClientSockets[i] == 0)
				{
					char addrBuff[32];
					InetNtop(AF_INET, &address, addrBuff, sizeof(addrBuff));
					listenerLog << "New connection:Client Number=" << i
						<< "\n\nAddress=" << addrBuff
						<< "\nPort=" << ntohs(address.sin_port) << endl;
					ClientSockets[i] = new_socket;
					++activeClients;

					strcpy_s(Buffer, "_join ");
					Buffer[6] = i + 49;	//get the ascii character for the number of the client (+1 for non-zero-indexed player numbers in main thread)
					Buffer[7] = '\0';
					listenerLog << "Adding to queue: " << Buffer << endl;
					locker.lock();
					messages.insert(messages.end(), Message(Buffer, i));
					locker.unlock();
					break;
				}
			}
			if (activeClients == maxClients)
			{
				closesocket(ListenSocket);
				listeningForNew = false;
			}
		}

		//check for network input from active clients
		for (int i = 0; i < maxClients; ++i)
		{
			if ((ClientSockets[i] != 0) && (FD_ISSET(ClientSockets[i], &readFDs)))
			{
				valRead = recv(ClientSockets[i], Buffer, 256, 0);
				if (valRead == 0)
				{
					//disconnected
					--activeClients;
					listenerLog << "Client " << i << "disconnected." << endl;
					strcpy_s(Buffer, "_leave ");
					Buffer[7] = i + 49;	//get the ascii character for the number of the client
					Buffer[8] = '\0';
					listenerLog << "Adding to queue: " << Buffer << endl;
					locker.lock();
					messages.insert(messages.end(), Message(Buffer, i));
					locker.unlock();
					closesocket(ClientSockets[i]);
					ClientSockets[i] = 0;
					if (activeClients == 0)
					{
						running = false;
					}
				}
				else
				{
					Buffer[valRead] = '\0';
					//add to queue
					listenerLog << "Adding to queue: " << Buffer << endl;
					locker.lock();
					messages.insert(messages.end(), Message(Buffer, i));
					locker.unlock();
				}
			}
		}
	}
	if (ListenSocket != 0)
	{
		closesocket(ListenSocket);
	}
	for (int i = 0; i < maxClients; ++i)
	{
		if (ClientSockets[i] != 0)
		{
			closesocket(ClientSockets[i]);
			ClientSockets[i] = 0;
		}
	}
	listenerLog.close();
}

//main thread
void NetworkServer::Broadcast(const char* message)
{
	serverfile << "Broadcasting: " << message << endl;
	for(int i = 0; i < maxClients; ++i)
	{
		if(ClientSockets[i] != 0)
			Send(message, i);
	}
}

//main thread
void NetworkServer::Send(const char* message, unsigned int client)
{
	if (ClientSockets[client] != 0)
	{
		serverfile << "Sending to client " << client << ": " << message << endl;
		strcpy_s(Buffer, message);
		send(ClientSockets[client], Buffer, 256, 0);
	}
	else
		serverfile << "Ignored sending \"" << message << "\" because the client isn't active." << endl;
}

DWORD WINAPI ThreadFunction(LPVOID Whatever)
{
	((NetworkServer*)Whatever)->Listen();
	return 0;
}
