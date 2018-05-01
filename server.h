#pragma once
#include "game.h"
#include "player.h"
#include "NetworkServer.h"

class server
{
public:
	server(unsigned short players);
	~server(void);
	void handleMessages();
	bool empty();
	bool isRunning();
	Game* getGame() { return &thegame; }
	NetworkServer* getNetwork() { return &network; }

private:
	Game thegame;
	NetworkServer network;
};
