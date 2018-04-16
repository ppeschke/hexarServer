#include <iostream>
#include <cmath>
using namespace std;

#include "NetworkServer.h"
#include "server.h"
#include "hexagon.h"
#include "rotator.h"
#include "game.h"
#include "action.h"

void play_game();
base* getHexagon(Game* thegame, int i, int p);
player* getPlayer(player players[], sockaddr_in addy);

NetworkServer networkServer;
server Server(&networkServer);
Game thegame(CLIENTS);

action parseMessage(char []);
float distance(float x1, float y1, float x2, float y2);
bool nextTo(base* a, base* b);
base* getHexagon(Game* thegame, int i, int p);
base* stronger(base* a, base* b);
bool next(Game* thegame, color c, int i, int p);
base* getItem(Game* thegame, int i, int p);
string toString(int);
void delay(int);

int main()
{
	int boardsize = 4;
	if(boardsize < thegame.playerNum * 2)
		boardsize = thegame.playerNum * 2;
	bool picked = false;	//indicates if they've picked their colors
	
	cout << "waiting for " << thegame.playerNum << " players to connect" << endl;
	while (networkServer.ClientCount() < CLIENTS && Server.isRunning())
	{}
	delay(1000);
	cout << "All clients connected... sending menu command" << endl;
	networkServer.Broadcast("_menu");
	//setup phase
	while(!picked && Server.isRunning())
	{
		Server.handleMessages();
		picked = true;
		for(int i = 0; i < thegame.playerNum; ++i)
		{
			if(thegame.players[i].c == white)
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
				thegame.objects.insert(thegame.objects.end(), temp);
				x -= 2.5;
			}
			count += (i + 1 < boardsize ? 1 : -1);
			rx = rx + (i + 1 < boardsize? 1.5f : -1.5f);
			rz += 2.0;
		}
	}

	//build and send setup string
	string msg = "_setup";
	msg += " " + toString(thegame.playerNum);
	msg += " " + toString(boardsize);
	Server.networkComponent->Broadcast(msg.c_str());
	cout << "done." << endl;

	cout << "Giving out land...";
	int i, p;
	base* temp;
	for(int j = 1; j <= thegame.playerNum; ++j)
	{
		switch(thegame.playerNum)
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
		msg += " " + toString((int)thegame.players[j - 1].c);
		Server.networkComponent->Broadcast(msg.c_str());
		msg = "_buy base " + toString(i);
		msg += " " + toString(p);
		msg += " " + toString((int)thegame.players[j - 1].c);
		Server.networkComponent->Broadcast(msg.c_str());
		temp = new rotator(getHexagon(&thegame, i, p)->x, getHexagon(&thegame, i, p)->z, thegame.players[j - 1].c, i, p, "base");
		thegame.objects.insert(thegame.objects.end(), temp);
	}
	for (int j = 0; j < CLIENTS; ++j)
	{
		thegame.players[j].peschkes = 88;
	}
	Server.networkComponent->Broadcast("_peschkes 88");
	cout << "done." << endl;
	delay(1000);
	
	play_game();
	
	//remain active until last player has left, then shut down the server
	while(!Server.empty())
	{
		Server.handleMessages();
	}
	
	return 0;
}

void delay(int time)
{
	DWORD delayer = GetTickCount();
	while (GetTickCount() < delayer + time) {}
}

void play_game()
{
	networkServer.Broadcast("_play");
	while(!thegame.over)
	{
		Server.handleMessages();
		if(thegame.turn > thegame.playerNum)
			thegame.turn = 1;
	}
}

void recalculateTurns()
{
	int actives = 0;
	for(int i = 0; i < CLIENTS; ++i)
	{
		if(thegame.players[i].active)
			++actives;
	}
	if(actives < 2)
		thegame.over = true;
	else
	{
		while(!thegame.players[thegame.turn - 1].active)
		{
			++thegame.turn;
			if(thegame.turn == CLIENTS)
				thegame.turn = 1;
		}
	}
	cout << "It's Player" << thegame.turn << "'s turn." << endl;
	return;
}

void recalculateMoney(player& p)
{
	list<base*> objs;
	for(list<base*>::iterator index = thegame.objects.begin(); index != thegame.objects.end(); ++index)
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
	return a;
}

void doAction(action& tempAction, unsigned int clientNum)
{
	string msg;
	if(tempAction.name == "_endturn")
	{
		recalculateMoney(thegame.players[thegame.turn - 1]);
		if (thegame.players[thegame.turn - 1].peschkes < 1)
		{
			Server.networkComponent->Send("Game over for you.", clientNum);
			thegame.players[thegame.turn - 1].active = false;
		}
		else
		{
			msg = "_peschkes ";
			msg += toString(thegame.players[thegame.turn - 1].peschkes);
			Server.networkComponent->Send(msg.c_str(), clientNum);
		}
		++(thegame.turn);
		msg = "player";
		msg += ((thegame.turn) - 1) + " has ended their turn.";
		Server.networkComponent->Broadcast(msg.c_str());
		if (thegame.turn > CLIENTS)
			thegame.turn = 1;
	}
	else if(tempAction.name == "_grab")
	{
		//make sure it's not taken and its next to the player's land and the player has enough peschkes to buy
		if(getHexagon(&thegame, tempAction.int1, tempAction.int2)->c == white && next(&thegame, thegame.players[thegame.turn - 1].c, tempAction.int1, tempAction.int2) && PLOTPRICE < thegame.players[thegame.turn - 1].peschkes)
		{
			thegame.players[thegame.turn - 1].peschkes -= PLOTPRICE;
			getHexagon(&thegame, tempAction.int1, tempAction.int2)->c = thegame.players[thegame.turn - 1].c;
			msg = "_peschkes ";
			msg += toString(thegame.players[thegame.turn - 1].peschkes);
			Server.networkComponent->Send(msg.c_str(), clientNum);
			msg = "_grab ";
			msg += toString(tempAction.int1);
			msg += " " + toString(tempAction.int2);
			msg += " " + toString((int)thegame.players[thegame.turn - 1].c);
			Server.networkComponent->Broadcast(msg.c_str());
		}
		else
		{
			Server.networkComponent->Send("Either you're broke, that land is already occupied or it's out of range.", clientNum);
		}
	}
	else if(tempAction.name == "_move")
	{
		base* temp = NULL;
		//make sure there's something there to move
		if(temp = getItem(&thegame, tempAction.int1, tempAction.int2))
		{
			//make sure the player owns the first coords
			if(thegame.players[thegame.turn - 1].c == temp->c)
			{
				base* temp2 = getItem(&thegame, tempAction.int3, tempAction.int4);
				//if there's something else there
				if(temp2 && (stronger(temp, temp2) != temp || temp2->c == thegame.players[thegame.turn - 1].c || thegame.players[thegame.turn - 1].peschkes <= PLOTPRICE))
				{
					Server.networkComponent->Send("You cannot move that object to that location.", clientNum);
				}
				else
				{
					base* item = getItem(&thegame, tempAction.int1, tempAction.int2);
					base* hex = getHexagon(&thegame, tempAction.int3, tempAction.int4);
					if (!(item && hex))
						cout << "PROBLEM MOVING!!!!" << endl;
					item->i = hex->i;
					item->p = hex->p;
					item->x = hex->x;
					item->z = hex->z;
					cout << "Player" << thegame.turn << " has moved a " << temp->_type << "." << endl;
					//if the player doesn't own the second coords
					if(thegame.players[thegame.turn - 1].c != getHexagon(&thegame, tempAction.int3, tempAction.int4)->c)
						thegame.players[thegame.turn - 1].peschkes -= PLOTPRICE;
					//player is implied by color of object moved, no need to send
					msg = "_peschkes ";
					msg += toString(thegame.players[thegame.turn - 1].peschkes);
					Server.networkComponent->Send(msg.c_str(), clientNum);
					msg = "_move " + toString(tempAction.int1);
					msg += " " + toString(tempAction.int2);
					msg += " " + toString(tempAction.int3);
					msg += " " + toString(tempAction.int4);
					Server.networkComponent->Broadcast(msg.c_str());
				}
			}
			else
			{
				Server.networkComponent->Send("You do not own that!", clientNum);
			}
		}
		else
			Server.networkComponent->Send("There's nothing to move at that location.", clientNum);
	}
	else if(tempAction.name == "_buy")
	{
		int price = (tempAction.item == "turret"? TURRETPRICE:(tempAction.item == "walker"? WALKERPRICE:BASEPRICE));
		if(price <= thegame.players[thegame.turn - 1].peschkes)
		{
			//make sure the player owns it, and make sure there's nothing on it
			if(getHexagon(&thegame, tempAction.int1, tempAction.int2)->c == thegame.players[thegame.turn - 1].c && !getItem(&thegame, tempAction.int1, tempAction.int2))
			{
				//player is implied by the color of the hexagon, no need to send that info
				thegame.players[thegame.turn - 1].peschkes -= price;
				msg = "_peschkes ";
				msg += toString(thegame.players[thegame.turn - 1].peschkes);
				Server.networkComponent->Send(msg.c_str(), clientNum);
				msg = "_buy ";
				msg += tempAction.item;
				msg += " " + toString(tempAction.int1);
				msg += " " + toString(tempAction.int2);
				Server.networkComponent->Broadcast(msg.c_str());
				cout << "Player" << thegame.turn << " has bought a " << tempAction.item << "." << endl;
				base* temp = new rotator(getHexagon(&thegame, tempAction.int1, tempAction.int2)->x, getHexagon(&thegame, tempAction.int1, tempAction.int2)->z, thegame.players[thegame.turn - 1].c, tempAction.int1, tempAction.int2, tempAction.item);
				thegame.objects.insert(thegame.objects.end(), temp);
				temp = NULL;
			}
			else
				Server.networkComponent->Send("You cannot put anything there.", clientNum);
		}
		else
		{
			msg = "You do not have enough money (";
			msg += thegame.players[thegame.turn - 1].peschkes + " peschkes)";
			Server.networkComponent->Send(msg.c_str(), clientNum);
		}
	}
	else if(tempAction.name == "_request")
	{
		bool found = false;
		string colors[] = {"white", "red", "green", "blue", "yellow", "orange", "cyan"};
		for(int i = 0; i < thegame.playerNum; ++i)
		{
			if(thegame.players[i].active && colors[(int)thegame.players[i].c] == tempAction.item)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			player* p = &thegame.players[clientNum];
			//granted
			msg = "_granted " + tempAction.item;
			Server.networkComponent->Send(msg.c_str(), clientNum);
			for(int i = 0; i < 7; ++i)
			{
				if(tempAction.item == colors[i])
				{
					p->c = (color)i;
					break;
				}
			}

			msg = "_taken " + tempAction.item;
			Server.networkComponent->Broadcast(msg.c_str());
			cout << tempAction.item << " has been selected" << endl;
		}
		else
		{
			//taken
			msg = "_taken " + tempAction.item;
			Server.networkComponent->Send(msg.c_str(), clientNum);
		}
	}
	else
	{
		Server.networkComponent->Send("Unsupported command!", clientNum);
	}
	return;
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