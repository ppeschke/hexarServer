#include "server.h"
#include "action.h"
#include "game.h"
#include "rotator.h"
#include <iostream>
using namespace std;

void nextTurn(server* Server);
action parseMessage(char []);
void doAction(action& tempAction, unsigned int clientA, server* Server);

server::server(unsigned short players) : thegame(players), network(players)
{
}

server::~server(void)
{
}

bool server::isRunning()
{
	return this->network.running;
}

void server::handleMessages()
{
	network.locker.lock();
	while(network.messages.size() > 0)
	{
		if((*network.messages.begin()).message[0] == '_')
		{
			action tempAction = parseMessage((*network.messages.begin()).message);
			doAction(tempAction, (*network.messages.begin()).fromPlayer, this);
		}
		else
		{
			//chat message
			string colorNames[7] = { "White", "Red", "Green", "Blue", "Yellow", "Orange", "Cyan" };
			Message firstMessage = (*network.messages.begin());
			color c = thegame.players[firstMessage.fromPlayer].c;
			string message;
			if(c != white)
			{
				//append color name
				message = colorNames[(int)c] + ": " + firstMessage.message;
			}
			else
				message = firstMessage.message;
			this->network.Broadcast(message.c_str());
		}
		network.messages.pop_front();
	}
	network.locker.unlock();
}

bool server::empty()
{
	return (network.ActiveClients() == 0);
}