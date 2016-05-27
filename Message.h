#pragma once
class Message
{
public:
	Message(char* Buffer, unsigned int fP);
	~Message();

	char message[256];
	unsigned int fromPlayer;
};

