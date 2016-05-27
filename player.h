#pragma once

#include "global.h"
#define _WINSOCKAPI_
#include <winsock2.h>
#include <stdio.h>

class player
{
public:
	player(void);
	~player(void);

	color c;
	bool active;
	int peschkes;
	//sockaddr_in addy;
};
