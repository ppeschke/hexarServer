#pragma once

#define _WINSOCKAPI_
#include <winsock2.h>
#include <stdio.h>

#include "game.h"
#include "player.h"

#pragma comment(lib, "Ws2_32.lib")

class server
{
public:
	server(unsigned int port);
	~server(void);
	bool recv();
	void handleMsgOld();
	void handleMsg(Game* thegame);
	void handleSMsg(Game* thegame);
	void broadcast(const char*);
	void send(const char* msg, sockaddr_in recpnt);
	bool empty();

	unsigned int server_port;
};
