#include "server.h"
#include "action.h"
#include "game.h"
#include "rotator.h"
#include <iostream>
using namespace std;

void recalculateTurns();
void recalculateMoney(player& p);
action parseMessage(char []);

//player* getPlayer(player players[], sockaddr_in addy);
void doAction(action& tempAction, unsigned int clientA);

//player* getPlayer(player players[], sockaddr_in addy)
//{
//	for(int i = 0; i < CLIENTS; ++i)
//	{
//		//if(addy.sin_addr.s_addr == ClientAddress[i].sin_addr.s_addr)
//			//return &(players[i]);
//	}
//	return NULL;
//}

server::server(NetworkServer* ns)
{
	networkComponent = ns;
}

server::~server(void)
{
}

void server::handleMessages()
{
	networkComponent->locker.lock();
	while(networkComponent->messages.size() > 0)
	{
		action tempAction = parseMessage(  (*networkComponent->messages.begin()).message  );
		doAction(tempAction, (*networkComponent->messages.begin()).fromPlayer);
		networkComponent->messages.pop_front();
		recalculateTurns();
	}
	networkComponent->locker.unlock();
}

bool server::empty()
{
	return (networkComponent->ClientCount() == 0);
}

//void server::handleMsg(Game* thegame)
//{
//	sockaddr_in clientA = IncomingAddress;
//	string msg;
//	if(Buffer[0] == 0)
//	{
//		//get Client and send goodbye to it
//		send(Buffer, clientA);
//		if(getPlayer(thegame->players, clientA))
//		{
//			getPlayer(thegame->players, clientA)->active = false;
//			ZeroMemory(getPlayer(thegame->players, clientA), sizeof(sockaddr_in));
//			for(int i = 0; i < 6; ++i)
//				if(ClientAddress[i].sin_addr.s_addr == IncomingAddress.sin_addr.s_addr)
//					ZeroMemory(&ClientAddress[i], sizeof(sockaddr_in));
//		}
//	}
//	else if(Buffer[0] == 1)
//	{
//		send("Server is full.", clientA);
//	}
//	else if(Buffer[0] == '_')
//	{
//		action tempAction = parseMessage(Buffer);
//		doAction(tempAction, clientA);
//	}
//	//recalculate turns, and actives
//	recalculateTurns();
//}

//void server::handleSMsg(Game* thegame)
//{
//	sockaddr_in clientA = IncomingAddress;
//	string msg;
//	if(Buffer[0] == 0)
//	{
//		//get Client and send goodbye to it
//		send(Buffer, clientA);
//		if(getPlayer(thegame->players, clientA))
//		{
//			player* p = getPlayer(thegame->players, clientA);
//			p->active = false;
//			//reset player's address
//			//ZeroMemory(&(p->addy), sizeof(sockaddr_in));
//			cout << "A player has disconnected" << endl;
//		}
//	}
//	else if(Buffer[0] == 1)
//	{
//		cout << "A player is attempting to join." << endl;
//		for(int i = 0; i < thegame->playerNum; i++)
//		{
//			if(!ClientAddress[i].sin_family)
//			{
//				ClientAddress[i] = IncomingAddress;
//				//aknowledge knock (come in)
//				sendto(Socket, Buffer, 64, 0, (sockaddr*)&ClientAddress[i], sizeof(sockaddr));
//				//put in players array
//				cout << "Success, we now have a player" << i + 1 << endl;
//				thegame->players[i].active = true;
//				break;
//			}
//		}
//	}
//	else if(Buffer[0] == '_')
//	{
//		action tempAction = parseMessage(Buffer);
//		doAction(tempAction, clientA);
//	}
//}