#include <iostream>
#include <cmath>
using namespace std;

#include "server.h"
#include "hexagon.h"
#include "rotator.h"
#include "action.h"

void play_game(server* Server);
void nextTurn(server* Server);
base* getHexagon(Game* thegame, int i, int p);
action parseMessage(char []);
float distance(float x1, float y1, float x2, float y2);
bool nextTo(base* a, base* b);
base* getHexagon(Game* thegame, int i, int p);
base* stronger(base* a, base* b);
bool next(Game* thegame, color c, int i, int p);
base* getItem(Game* thegame, int i, int p);
string toString(int);

int main()
{
	server Server(1);
	if (!Server.isRunning())
	{
		return (1);
	}
	Game* thegame = Server.getGame();
	int boardsize = 4;
	if(boardsize < thegame->playerNum * 2)
		boardsize = thegame->playerNum * 2;
	bool picked = false;	//indicates if they've picked their colors
	
	cout << "waiting for " << thegame->playerNum << " players to connect" << endl;
	while (Server.getNetwork()->ActiveClients() < Server.getNetwork()->MaxClients() && Server.isRunning())
	{
		Server.handleMessages();
	}
	Server.handleMessages();
	if (!Server.isRunning())
	{
		cout << "ERROR: Out of connection phase, and server is no longer running." << endl;
		return(1);
	}
	cout << "All clients connected... sending menu command" << endl;
	Server.getNetwork()->Broadcast("_menu");
	//setup phase
	while(!picked && Server.isRunning())
	{
		Server.handleMessages();
		picked = true;
		for(int i = 0; i < thegame->playerNum; ++i)
		{
			if(thegame->players[i].c == white)
				picked = false;
		}
	}
	cout << "Setting up the board...";
	/*general board setup block*/
	{
		int count = boardsize;
		float x, rx, rz;
		rx = 0.0f + 1.5f * (boardsize - 1);
		rz = 0.0f - 2.0f * (boardsize - 1);
		base* temp;
		for(int i = 0; i < boardsize * 2 - 1; ++i)
		{
			x = rx;
			for(int p = 0; p < count; ++p)
			{
				temp = new hexagon(x, rz, white, i, p);
				thegame->objects.insert(thegame->objects.end(), temp);
				x -= 2.5;
			}
			count += (i + 1 < boardsize ? 1 : -1);
			rx = rx + (i + 1 < boardsize? 1.5f : -1.5f);
			rz += 2.0;
		}
	}

	//build and send setup string
	string msg = "_setup";
	msg += " " + toString(thegame->playerNum);
	msg += " " + toString(boardsize);
	Server.getNetwork()->Broadcast(msg.c_str());
	cout << "done." << endl;

	cout << "Giving out land...";
	int i, p;
	base* temp;
	for(int j = 1; j <= thegame->playerNum; ++j)
	{
		switch(thegame->playerNum)
		{
		case 1:
			i = 0;
			p = 0;
			break;
		case 2:
			if(j == 1)
			{
				i = 0;
				p = 0;
			}
			else
			{
				i = boardsize * 2 - 2;
				p = boardsize - 1;
			}
			break;
		case 3:
			if(j == 1)
			{
				i = 0;
				p = 0;
			}
			else if(j == 2)
			{
				i = boardsize - 1;
				p = boardsize * 2 - 2;
			}
			else
			{
				i = 0;
				p = boardsize * 2 - 2;
			}
			break;
		case 4:
			if(j == 1)
			{
				i = 0;
				p = 0;
			}
			else if(j == 2)
			{
				i = 0;
				p = boardsize - 1;
			}
			else if(j == 3)
			{
				i = boardsize * 2 - 2;
				p = boardsize * 2 - 1;
			}
			else
			{
				i = 0;
				p = boardsize * 2 - 2;
			}
			break;
		case  5:
			if(j == 1)
			{
				i = 0;
				p = 0;
			}
			else if(j == 2)
			{
				i = 0;
				p = boardsize - 1;
			}
			else if(j == 3)
			{
				i = boardsize - 2;
				p = boardsize - 1;
			}
			else if(j == 4)
			{
				i = boardsize * 2 - 2;
				p = boardsize * 2 - 2;
			}
			else
			{
				i = boardsize * 2 - 2;
				p = 0;
			}
			break;
		case 6:
			if(j == 1)
			{
				i = 0;
				p = 0;
			}
			else if(j == 2)
			{
				i = 0;
				p = boardsize - 1;
			}
			else if(j == 3)
			{
				i = boardsize - 2;
				p = boardsize - 1;
			}
			else if(j == 4)
			{
				i = boardsize * 2 - 2;
				p = boardsize * 2 - 2;
			}
			else if(j == 5)
			{
				i = boardsize * 2 - 2;
				p = 0;
			}
			else
			{
				i = boardsize - 1;
				p = 0;
			}
			break;
		}
		msg = "_grab";
		msg += " " + toString(i);
		msg += " " + toString(p);
		msg += " " + toString((int)thegame->players[j - 1].c);
		Server.getNetwork()->Broadcast(msg.c_str());
		msg = "_buy base " + toString(i);
		msg += " " + toString(p);
		msg += " " + toString((int)thegame->players[j - 1].c);
		Server.getNetwork()->Broadcast(msg.c_str());
		temp = new rotator(getHexagon(thegame, i, p)->x, getHexagon(thegame, i, p)->z, thegame->players[j - 1].c, i, p, "base");
		thegame->objects.insert(thegame->objects.end(), temp);
	}
	for (int j = 0; j < Server.getNetwork()->MaxClients(); ++j)
	{
		thegame->players[j].peschkes = 88;
	}
	Server.getNetwork()->Broadcast("_peschkes 88");
	cout << "done." << endl;
	
	play_game(&Server);
	
	//remain active until last player has left, then shut down the server
	while(!Server.empty())
	{
		Server.handleMessages();
	}
	
	return 0;
}

void play_game(server* Server)
{
	Server->getNetwork()->Broadcast("_play");
	while(!Server->getGame()->over)
	{
		Server->handleMessages();
		nextTurn(Server);
		if(Server->getGame()->turn > Server->getGame()->playerNum)
			Server->getGame()->turn = 1;
	}
}

void nextTurn(server* Server)
{
	int actives = 0;
	for(int i = 0; i < Server->getNetwork()->MaxClients(); ++i)
	{
		if(Server->getGame()->players[i].active)
			++actives;
	}
	if(actives < 2)
		Server->getGame()->over = true;
	else
	{
		while(!Server->getGame()->players[Server->getGame()->turn - 1].active)
		{
			++Server->getGame()->turn;
			if(Server->getGame()->turn == Server->getNetwork()->MaxClients())
				Server->getGame()->turn = 1;
		}
	}
	cout << "It's Player" << Server->getGame()->turn << "'s turn." << endl;
	return;
}

void recalculateMoney(player& p, server* Server)
{
	list<base*> objs;
	for(list<base*>::iterator index = Server->getGame()->objects.begin(); index != Server->getGame()->objects.end(); ++index)
	{
		if((*index)->c == p.c)
			objs.insert(objs.end(), (*index));
	}
	for(list<base*>::iterator index = objs.begin(); index != objs.end(); ++index)
	{
		if((*index)->_type == "hexagon")
			p.peschkes += PLOTREVENUE;
		else if((*index)->_type == "base")
			p.peschkes += BASEREVENUE;
		else if((*index)->_type == "turret")
			p.peschkes -= TURRETREQ;
		else if((*index)->_type == "walker")
			p.peschkes -= WALKERREQ;
	}
}

action parseMessage(char buffer[])
{
	action a;
	string b = buffer;
	int index = b.find(" ");
	int index2;
	a.name = b.substr(0, index);
	if(a.name == "_grab")
	{
		index2 = b.find(" ", index + 1);
		a.int1 = atoi(b.substr(index, index2).c_str());
		a.int2 = atoi(b.substr(index2).c_str());
	}
	else if(a.name == "_move")
	{
		index += 1;
		index2 = b.find_last_not_of(" ", index);
		a.int1 = atoi(b.substr(index, (index2 - index) + 1).c_str());

		index = index2 + 2;
		index2 = b.find_last_not_of(" ", index);
		a.int2 = atoi(b.substr(index, (index2 - index) + 1).c_str());

		index = index2 + 2;
		index2 = b.find_last_not_of(" ", index);
		a.int3 = atoi(b.substr(index, (index2 - index) + 1).c_str());

		index = index2 + 2;
		index2 = b.find_last_not_of(" ", index);
		a.int4 = atoi(b.substr(index, (index2 - index) + 1).c_str());
	}
	else if(a.name == "_buy")
	{
		index2 = b.find(" ", index + 1);
		a.item = b.substr(index + 1, index2 - index - 1);
		index = index2 + 1;
		index2 = b.find(" ", index + 1);
		a.int1 = atoi(b.substr(index, index2 - index).c_str());
		index = index2 + 1;
		index2 = b.find(" ", index);
		a.int2 = atoi(b.substr(index).c_str());
	}
	else if(a.name == "_endturn")
	{}
	else if(a.name == "_request")
	{
		a.item = b.substr(index + 1);
	}
	else if (a.name == "_join")
	{
		index2 = b.find(" ", index + 1);
		a.int1 = atoi(b.substr(index + 1, index2 - index - 1).c_str());
	}
	else if (a.name == "_leave")
	{
		index2 = b.find(" ", index + 1);
		a.int1 = atoi(b.substr(index + 1, index2 - index - 1).c_str());
	}
	return a;
}



float distance(float x1, float y1, float x2, float y2)
{
	return sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2));
}

bool nextTo(base* a, base* b)
{
	return distance(a->x, a->z, b->x, b->z) < 3.0f;
}

base* getHexagon(Game* thegame, int i, int p)
{
	for(list<base*>::iterator index = thegame->objects.begin(); index != thegame->objects.end(); ++index)
	{
		if((*index)->coords(i, p))
			return (*index);
	}
	return NULL;
}

base* getItem(Game* thegame, int i, int p)
{
	for(list<base*>::reverse_iterator rindex = thegame->objects.rbegin(); rindex != thegame->objects.rend(); ++rindex)
	{
		if((*rindex)->rcoords(i, p))
			return (*rindex);
	}
	return NULL;
}

bool next(Game* thegame, color c, int i, int p)
{
	list<base*> hexs;
	base* temp = getHexagon(thegame, i, p);
	for(list<base*>::iterator index = thegame->objects.begin(); index != thegame->objects.end(); ++index)
	{
		if(nextTo(temp, (*index)) && (*index)->c == c)
			return true;
	}
	return false;
}

base* stronger(base* a, base* b)
{
	if(b->type() == "walker")
	{
		if(a->type() != "walker")
			return b;
		else
			return a;
	}
	else if(b->type() == "base")
	{
		if(a->type() != "walker")
			return b;
		else
			return a;
	}
	else
	{
		if(a->type() == "walker" || a->type() == "base")
			return a;
		else
			return b;
	}
}

string toString(int i)
{
	char temp[10];
	_itoa_s(i, temp, 10);
	return (string)temp;
}