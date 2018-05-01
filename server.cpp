#include "server.h"
#include "action.h"
#include "game.h"
#include "rotator.h"
#include <iostream>
using namespace std;

void recalculateTurns(server* Server);
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
		action tempAction = parseMessage(  (*network.messages.begin()).message  );
		doAction(tempAction, (*network.messages.begin()).fromPlayer, this);
		network.messages.pop_front();
		recalculateTurns(this);
	}
	network.locker.unlock();
}

bool server::empty()
{
	return (network.ClientCount() == 0);
}