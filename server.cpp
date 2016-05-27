#include "server.h"
#include "action.h"
#include "game.h"
#include "rotator.h"
#include <iostream>
using namespace std;

void recalculateTurns();
void recalculateMoney(player& p);
action parseMessage(char []);

WSADATA Winsock;
SOCKET Socket;
sockaddr_in ServerAddress;
sockaddr_in IncomingAddress;	//Contains the address of the sending client
sockaddr_in ClientAddress[6];	//Stores the client's addresses
char Buffer[64];
int AddressLen = sizeof(IncomingAddress);

player* getPlayer(player players[], sockaddr_in addy);
void doAction(action& tempAction, sockaddr_in clientA);

player* getPlayer(player players[], sockaddr_in addy)
{
	for(int i = 0; i < 6; ++i)
	{
		if(addy.sin_addr.s_addr == ClientAddress[i].sin_addr.s_addr)
			return &(players[i]);
	}
	return NULL;
}

server::server(unsigned int port)
{
	server_port = port;
	WSAStartup(MAKEWORD(2, 2), &Winsock);

    if(LOBYTE(Winsock.wVersion) != 2 || HIBYTE(Winsock.wVersion) != 2)
    {
        WSACleanup();
        return;
    }

    // Make the Socket
    Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Input server information and bind it to the socket
    ZeroMemory(&ServerAddress, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
	ServerAddress.sin_port = server_port;
    bind(Socket, (sockaddr*)&ServerAddress, sizeof(ServerAddress));
}

server::~server(void)
{
	WSACleanup();
}

bool server::recv()
{
	return recvfrom(Socket, Buffer, 64, 0, (sockaddr*)&IncomingAddress, &AddressLen) != 0;
}

bool server::empty()
{
	bool found = false;
	for(int i = 0; i < 6; ++i)
	{
		if(!ClientAddress[i].sin_family)
			found = true;
	}
	return !found;
}

void server::handleMsgOld()
{
	//If the packet is a knock, add the client's address to ClientAddress
	if(Buffer[0] == 1)
	{
		for(int i = 0; i < 6; i++)
		{
			if(!ClientAddress[i].sin_family)
			{
				ClientAddress[i] = IncomingAddress;
				//aknowledge knock (come in)
				sendto(Socket, Buffer, 64, 0, (sockaddr*)&ClientAddress[i], sizeof(sockaddr));
				break;
			}
		}
		return;
	}
	else if(Buffer[0] == 0)	//If a client has quit, remove that client's address from ClientAddress
	{
		for(int i = 0; i < 6; i++)
		{
		   if(ClientAddress[i].sin_addr.s_addr == IncomingAddress.sin_addr.s_addr)
				ZeroMemory(&ClientAddress[i], sizeof(sockaddr_in));
		}
	}
	else
	{
		//Display the message and broadcast it to all active clients
		Buffer[63] = '\0';    //Always end the packet with this
		printf("Broadcasting: ");
		printf(Buffer);
		printf("\n");
		//broadcast to all clients
		for(int i = 0; i < 6; i++)
		{
			if(ClientAddress[i].sin_family)
				sendto(Socket, Buffer, 64, 0, (sockaddr*)&ClientAddress[i], sizeof(sockaddr));
		}
	}
}

void server::handleMsg(Game* thegame)
{
	sockaddr_in clientA = IncomingAddress;
	string msg;
	if(Buffer[0] == 0)
	{
		//get Client and send goodbye to it
		send(Buffer, clientA);
		if(getPlayer(thegame->players, clientA))
		{
			getPlayer(thegame->players, clientA)->active = false;
			ZeroMemory(getPlayer(thegame->players, clientA), sizeof(sockaddr_in));
			for(int i = 0; i < 6; ++i)
				if(ClientAddress[i].sin_addr.s_addr == IncomingAddress.sin_addr.s_addr)
					ZeroMemory(&ClientAddress[i], sizeof(sockaddr_in));
		}
	}
	else if(Buffer[0] == 1)
	{
		send("Server is full.", clientA);
	}
	else if(Buffer[0] == '_')
	{
		action tempAction = parseMessage(Buffer);
		doAction(tempAction, clientA);
	}
	//recalculate turns, and actives
	recalculateTurns();
}

void server::handleSMsg(Game* thegame)
{
	sockaddr_in clientA = IncomingAddress;
	string msg;
	if(Buffer[0] == 0)
	{
		//get Client and send goodbye to it
		send(Buffer, clientA);
		if(getPlayer(thegame->players, clientA))
		{
			player* p = getPlayer(thegame->players, clientA);
			p->active = false;
			//reset player's address
			//ZeroMemory(&(p->addy), sizeof(sockaddr_in));
			cout << "A player has disconnected" << endl;
		}
	}
	else if(Buffer[0] == 1)
	{
		cout << "A player is attempting to join." << endl;
		for(int i = 0; i < thegame->playerNum; i++)
		{
			if(!ClientAddress[i].sin_family)
			{
				ClientAddress[i] = IncomingAddress;
				//aknowledge knock (come in)
				sendto(Socket, Buffer, 64, 0, (sockaddr*)&ClientAddress[i], sizeof(sockaddr));
				//put in players array
				cout << "Success, we now have a player" << i + 1 << endl;
				thegame->players[i].active = true;
				break;
			}
		}
	}
	else if(Buffer[0] == '_')
	{
		action tempAction = parseMessage(Buffer);
		doAction(tempAction, clientA);
	}
}

void server::broadcast(const char* msg)
{
	cout << "Broadcasting: " << msg << endl;
	for(int i = 0; i < 6; i++)
	{
		if(ClientAddress[i].sin_family)
			sendto(Socket, msg, 64, 0, (sockaddr*)&ClientAddress[i], sizeof(sockaddr));
	}
}

void server::send(const char* msg, sockaddr_in recpnt)
{
	sendto(Socket, msg, 64, 0, (sockaddr*)&recpnt, sizeof(sockaddr));
	cout << "Sending: " << msg << endl;
}