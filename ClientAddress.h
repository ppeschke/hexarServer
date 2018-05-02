#pragma once

#define _WINSOCKAPI_
#include <WinSock2.h>

class ClientAddress
{
public:
	ClientAddress();
	~ClientAddress();

	sockaddr_in address;
	bool active;
};

