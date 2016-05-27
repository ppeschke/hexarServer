#include "Message.h"
#include <cstring>
using namespace std;


Message::Message(char* buffer, unsigned int fP)
{
	strcpy_s(message, 256, buffer);
	fromPlayer = fP;
}


Message::~Message()
{
}
