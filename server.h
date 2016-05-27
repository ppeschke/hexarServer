#pragma once
#include "game.h"
#include "player.h"
#include "NetworkServer.h"

class server
{
public:
	NetworkServer* networkComponent;
	server(NetworkServer* ns);
	~server(void);
	void handleMessages();
	bool empty();
};
