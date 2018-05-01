#include <iostream>
using namespace std;

#include "action.h"
#include "server.h"
#include "rotator.h"

float distance(float x1, float y1, float x2, float y2);
bool nextTo(base* a, base* b);
base* getHexagon(Game* thegame, int i, int p);
base* stronger(base* a, base* b);
bool next(Game* thegame, color c, int i, int p);
base* getItem(Game* thegame, int i, int p);
void recalculateMoney(player& p, server* Server);
string toString(int);

void doAction(action& tempAction, unsigned int clientNum, server* Server)
{
	Game* thegame = Server->getGame();
	string msg;
	if (tempAction.name == "_endturn")
	{
		recalculateMoney(thegame->players[thegame->turn - 1], Server);
		if (thegame->players[thegame->turn - 1].peschkes < 1)
		{
			Server->getNetwork()->Send("Game over for you.", clientNum);
			thegame->players[thegame->turn - 1].active = false;
		}
		else
		{
			msg = "_peschkes ";
			msg += toString(thegame->players[thegame->turn - 1].peschkes);
			Server->getNetwork()->Send(msg.c_str(), clientNum);
		}
		++(thegame->turn);
		msg = "player";
		msg += ((thegame->turn) - 1) + " has ended their turn.";
		Server->getNetwork()->Broadcast(msg.c_str());
		if (thegame->turn > Server->getNetwork()->MaxClients())
			thegame->turn = 1;
	}
	else if (tempAction.name == "_grab")
	{
		//make sure it's not taken and its next to the player's land and the player has enough peschkes to buy
		if (getHexagon(thegame, tempAction.int1, tempAction.int2)->c == white && next(thegame, thegame->players[thegame->turn - 1].c, tempAction.int1, tempAction.int2) && PLOTPRICE < thegame->players[thegame->turn - 1].peschkes)
		{
			thegame->players[thegame->turn - 1].peschkes -= PLOTPRICE;
			getHexagon(thegame, tempAction.int1, tempAction.int2)->c = thegame->players[thegame->turn - 1].c;
			msg = "_peschkes ";
			msg += toString(thegame->players[thegame->turn - 1].peschkes);
			Server->getNetwork()->Send(msg.c_str(), clientNum);
			msg = "_grab ";
			msg += toString(tempAction.int1);
			msg += " " + toString(tempAction.int2);
			msg += " " + toString((int)thegame->players[thegame->turn - 1].c);
			Server->getNetwork()->Broadcast(msg.c_str());
		}
		else
		{
			Server->getNetwork()->Send("Either you're broke, that land is already occupied or it's out of range.", clientNum);
		}
	}
	else if (tempAction.name == "_move")
	{
		base* temp = NULL;
		//make sure there's something there to move
		if (temp = getItem(thegame, tempAction.int1, tempAction.int2))
		{
			//make sure the player owns the first coords
			if (thegame->players[thegame->turn - 1].c == temp->c)
			{
				base* temp2 = getItem(thegame, tempAction.int3, tempAction.int4);
				//if there's something else there
				if (temp2 && (stronger(temp, temp2) != temp || temp2->c == thegame->players[thegame->turn - 1].c || thegame->players[thegame->turn - 1].peschkes <= PLOTPRICE))
				{
					Server->getNetwork()->Send("You cannot move that object to that location.", clientNum);
				}
				else
				{
					base* item = getItem(thegame, tempAction.int1, tempAction.int2);
					base* hex = getHexagon(thegame, tempAction.int3, tempAction.int4);
					if (!(item && hex))
						cout << "PROBLEM MOVING!!!!" << endl;
					item->i = hex->i;
					item->p = hex->p;
					item->x = hex->x;
					item->z = hex->z;
					cout << "Player" << thegame->turn << " has moved a " << temp->_type << "." << endl;
					//if the player doesn't own the second coords
					if (thegame->players[thegame->turn - 1].c != getHexagon(thegame, tempAction.int3, tempAction.int4)->c)
						thegame->players[thegame->turn - 1].peschkes -= PLOTPRICE;
					//player is implied by color of object moved, no need to send
					msg = "_peschkes ";
					msg += toString(thegame->players[thegame->turn - 1].peschkes);
					Server->getNetwork()->Send(msg.c_str(), clientNum);
					msg = "_move " + toString(tempAction.int1);
					msg += " " + toString(tempAction.int2);
					msg += " " + toString(tempAction.int3);
					msg += " " + toString(tempAction.int4);
					Server->getNetwork()->Broadcast(msg.c_str());
				}
			}
			else
			{
				Server->getNetwork()->Send("You do not own that!", clientNum);
			}
		}
		else
			Server->getNetwork()->Send("There's nothing to move at that location.", clientNum);
	}
	else if (tempAction.name == "_buy")
	{
		int price = (tempAction.item == "turret" ? TURRETPRICE : (tempAction.item == "walker" ? WALKERPRICE : BASEPRICE));
		if (price <= thegame->players[thegame->turn - 1].peschkes)
		{
			//make sure the player owns it, and make sure there's nothing on it
			if (getHexagon(thegame, tempAction.int1, tempAction.int2)->c == thegame->players[thegame->turn - 1].c && !getItem(thegame, tempAction.int1, tempAction.int2))
			{
				//player is implied by the color of the hexagon, no need to send that info
				thegame->players[thegame->turn - 1].peschkes -= price;
				msg = "_peschkes ";
				msg += toString(thegame->players[thegame->turn - 1].peschkes);
				Server->getNetwork()->Send(msg.c_str(), clientNum);
				msg = "_buy ";
				msg += tempAction.item;
				msg += " " + toString(tempAction.int1);
				msg += " " + toString(tempAction.int2);
				Server->getNetwork()->Broadcast(msg.c_str());
				cout << "Player" << thegame->turn << " has bought a " << tempAction.item << "." << endl;
				base* temp = new rotator(getHexagon(thegame, tempAction.int1, tempAction.int2)->x, getHexagon(thegame, tempAction.int1, tempAction.int2)->z, thegame->players[thegame->turn - 1].c, tempAction.int1, tempAction.int2, tempAction.item);
				thegame->objects.insert(thegame->objects.end(), temp);
				temp = NULL;
			}
			else
				Server->getNetwork()->Send("You cannot put anything there.", clientNum);
		}
		else
		{
			msg = "You do not have enough money (";
			msg += thegame->players[thegame->turn - 1].peschkes + " peschkes)";
			Server->getNetwork()->Send(msg.c_str(), clientNum);
		}
	}
	else if (tempAction.name == "_request")
	{
		bool found = false;
		string colors[] = { "white", "red", "green", "blue", "yellow", "orange", "cyan" };
		for (int i = 0; i < thegame->playerNum; ++i)
		{
			if (thegame->players[i].active && colors[(int)thegame->players[i].c] == tempAction.item)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			player* p = &thegame->players[clientNum];
			//granted
			msg = "_granted " + tempAction.item;
			Server->getNetwork()->Send(msg.c_str(), clientNum);
			for (int i = 0; i < 7; ++i)
			{
				if (tempAction.item == colors[i])
				{
					p->c = (color)i;
					break;
				}
			}

			msg = "_taken " + tempAction.item;
			Server->getNetwork()->Broadcast(msg.c_str());
			cout << tempAction.item << " has been selected" << endl;
		}
		else
		{
			//taken
			msg = "_taken " + tempAction.item;
			Server->getNetwork()->Send(msg.c_str(), clientNum);
		}
	}
	else
	{
		Server->getNetwork()->Send("Unsupported command!", clientNum);
	}
	return;
}